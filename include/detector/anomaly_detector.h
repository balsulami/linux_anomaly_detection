//
// Created by root on 9/21/15.
//

#ifndef HIGHPERFORMANCELINUXSENSORS_ANOMALYDETECTOR_H
#define HIGHPERFORMANCELINUXSENSORS_ANOMALYDETECTOR_H

#include "stream/base_stream.h"
#include "sensor/parallel_worker.h"
#include <algorithm>
#include "utils/types.h"
#include "pipeline_estimator.h"
#include "utils/sys_record.h"
#include <unordered_map>
#include <thread>
#include <mutex>
#include <chrono>
#include "queue/message_queue.h"
#include "sensor/multithreaded_sensor.h"

enum class EVENT_TYPE {
    Normal, Anomalous
};

#ifdef DEBUG_LOG
#include<fstream>
std::ofstream traces("/home/bms/projects/linux_anomaly_detector/out/traces.txt");
std::ofstream output("/home/bms/projects/linux_anomaly_detector/out/logs.txt");
#endif


template<typename T>
class AnomalyDetector {
    typedef std::unordered_map<pid_t, std::vector<syscall_t>> PIDTraces;
    typedef message_queue<std::vector<T> *> _out_type;
    typedef void(*_callback)(AnomalyDetector * ptr) ;

public:
    AnomalyDetector(int predict_interval = 1, int train_interval = 5, int trace_len = 10)
            : _predict_interval(predict_interval), _train_interval(train_interval), _trace_len(trace_len) {

    }

    void start() {
        linux_sensor.config_events(true, false);
        linux_sensor.run();

        cout << "The detector is listening for " << _train_interval << " mins ... \n";
        cout.flush();
        ParallelWorker _train_worker;
        _train_worker.start(AnomalyDetector::_extract_traces, this, nullptr);
        std::this_thread::sleep_for(std::chrono::minutes(_train_interval));
        linux_sensor.stop_trace();
        result_queue().enqueue(nullptr);
        _train(this);

        linux_sensor.start_trace();
        _worker.start(AnomalyDetector::_extract_traces, this,AnomalyDetector::_detect);
        cout << "The detector is running\n";

    }

    _out_type & result_queue(){
        return linux_sensor.result_queue();
    }

    void stop(){
        linux_sensor.stop();
        result_queue().enqueue(nullptr);
    }

private:
    int _predict_interval, _train_interval, _trace_len;
    pipeline_estimator _detector;
    PIDTraces _current_traces;
    ParallelWorker _worker;
    multithreaded_sensor<T> linux_sensor;

    void static _extract_traces(AnomalyDetector *ptr,_callback callback) { //T1 & inQ,T2 & outQ,DataAggregator<T3> & _data,EventsWatch & watch ){
        while (true) {
            if(callback != nullptr){
                cout << "The detector is listening for " << ptr->_predict_interval << " mins ... \n";
                std::this_thread::sleep_for(std::chrono::minutes(ptr->_predict_interval));
            }
            try {
                std::vector<T> *events[1024];
                size_t count = ptr->result_queue().wait_dequeue_bulk(events, BULK_CAPACITY);

                for (size_t i = 0; i < count; i++) {
                    auto current = events[i];
                    if (current != nullptr) {
                        _append_traces(ptr->_current_traces, *current);
                        delete current;
                    }
                    else {
                        return;
                    }
                }
                if(callback != nullptr)
                    callback(ptr);
                cout.flush();
            }
            catch (std::exception &e) {
                std::cout << e.what() << "\n";
            }
        }
    }

    void static _append_traces(PIDTraces &_current_traces, std::vector<T> &records) {
        for (auto iter = records.begin(); iter != records.end(); iter++) {
            auto it = _current_traces.find(iter->_pid);

            if (it != _current_traces.end()) {
                it->second.push_back(iter->_syscall);
            }
            else {
                std::vector<syscall_t> sys_traces;
                sys_traces.reserve(500);
                sys_traces.push_back(iter->_syscall);
                _current_traces.emplace(iter->_pid, std::move(sys_traces));
            }
        }
    }

    static void _train(AnomalyDetector *ptr) {
        TraceList train_traces;
        std::vector<pid_t> train_pids;
        cout << "Extracting traces for training ... \n";
        _filter(ptr->_current_traces, ptr->_trace_len, train_pids, train_traces);
        // debug message
        cout << "training the detector ...." << "\n";
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        ptr->_detector.train(train_traces);
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto duration = duration_cast<minutes>(t2 - t1).count();
        cout << "****** " << duration << " mins to train the detector\n";
        #ifdef DEBUG_LOG
        std::sort(train_pids.begin(),train_pids.end());
        int i=0;
        output<<"trained pids=\n\t";
        traces<<"trained pids=\n\t";
        for(auto & pid:train_pids) {
            output <<"\t"<< pid <<"=="<<train_traces[i].size()<<"==>"<<train_traces[i]<<"\n";
            traces <<"\t"<< pid <<"=="<<train_traces[i].size()<<"==>"<<train_traces[i]<<"\n";
            i++;
        }
        output.flush();
        traces.flush();
        #endif
    }

    static void _detect(AnomalyDetector *ptr) {
        cout << "Extracting traces for predictions ... \n";
        TraceList pred_traces;
        std::vector<pid_t> pred_pids;
        _filter(ptr->_current_traces, ptr->_trace_len, pred_pids, pred_traces);
        PredictLists predictions = ptr->_detector.predict(pred_traces);
        cout << "The detection results are :\n";
        long normal_pids_size = std::count_if(predictions.begin(), predictions.end(),
                                         [](double pred) { return pred > 0.0; });
        cout << "\tnormal pids=" << normal_pids_size << "\n";
        cout << "\tanomlous pids=" << (predictions.size() - normal_pids_size) << "\n";

#ifdef DEBUG_LOG

        int i=0;
        traces<<"predict pids=\n";
        for(auto & pid:pred_pids) {
            traces <<"\t" << pid <<"=="<<predictions[i]<<"=="<<pred_traces[i].size()<<"==>"<<pred_traces[i]<<"\n";
            i++;
        }
        traces.flush();

        output << "Extracting traces for predictions ... \n";
        output << "The detection results are :\n";
        std::vector<std::tuple<pid_t,double>> normal_pids,anomlous_pids;
        for(int i=0;i<predictions.size();i++){
            if(predictions[i] > 0)
                normal_pids.emplace_back(pred_pids[i],predictions[i]);
            else
                anomlous_pids.emplace_back(pred_pids[i],predictions[i]);
        }
        output << "\tnormal pids=" << normal_pids.size() << "\n";
        if(normal_pids.size() > 0) {
            auto max = max_element(normal_pids.begin(), normal_pids.end(),
                                   [](tuple<pid_t, double> a, tuple<pid_t, double> b) {
                                       return std::get<1>(a) < std::get<1>(b);
                                   });
            int pos = find(pred_pids.begin(), pred_pids.end(), std::get<0>(*max)) - pred_pids.begin();
            output << "\t\tmax pid=" << std::get<0>(*max)<<","<<pred_traces[pos].size() << " value =" << std::get<1>(*max) << "\n";
            auto min = min_element(normal_pids.begin(), normal_pids.end(),
                                   [](tuple<pid_t, double> a, tuple<pid_t, double> b) {
                                       return std::get<1>(a) < std::get<1>(b);
                                   });
            pos = find(pred_pids.begin(), pred_pids.end(), std::get<0>(*min)) - pred_pids.begin();
            output << "\t\tmin pid=" << std::get<0>(*min)<<","<<pred_traces[pos].size() << " value =" << std::get<1>(*min) << "\n";
        }

        output << "\tanomlous pids=" << anomlous_pids.size() << "\n";
        if(anomlous_pids.size()>0) {
            auto max = max_element(anomlous_pids.begin(), anomlous_pids.end(),
                              [](tuple<pid_t, double> a, tuple<pid_t, double> b) {
                                  return std::get<1>(a) < std::get<1>(b);
                              });
            int pos = find(pred_pids.begin(), pred_pids.end(), std::get<0>(*max)) - pred_pids.begin();
            output << "\t\tmax pid=" << std::get<0>(*max)<<","<<pred_traces[pos].size() << " value =" << std::get<1>(*max) << "\n";
            auto min = min_element(anomlous_pids.begin(), anomlous_pids.end(),
                              [](tuple<pid_t, double> a, tuple<pid_t, double> b) {
                                  return std::get<1>(a) < std::get<1>(b);
                              });
            pos = find(pred_pids.begin(), pred_pids.end(), std::get<0>(*min)) - pred_pids.begin();
            output << "\t\tmin pid=" << std::get<0>(*min)<<","<<pred_traces[pos].size() << " value =" << std::get<1>(*min) << "\n";
        }
        output.flush();
#endif
    }

    static void _filter(PIDTraces &traces, int min_len, std::vector<pid_t> &extract_pids,
                        std::vector<std::vector<syscall_t>> &extract_traces) {
        size_t min = std::numeric_limits<size_t>::max(), max = std::numeric_limits<size_t>::min(), sum = 0, count = 0;
        for (auto &trace: traces) {
            if (trace.second.size() > min_len) {
                if (trace.second.size() < min)
                    min = trace.second.size();
                if (trace.second.size() > max)
                    max = trace.second.size();
                sum += trace.second.size();

                extract_pids.push_back(trace.first);
                extract_traces.push_back(std::move(trace.second));
            }
        }
#ifdef DEBUG_LOG
        output << "\t# " << extract_pids.size() << " traces is used" << "\n";
        if (extract_pids.size() > 0) {
            output << "\tMaximum trace length = " << max << "\n";
            output << "\tMinimum trace length = " << min << "\n";
            output << "\tAverage trace length = " << sum / extract_pids.size() << "\n";
//            for (auto &pid: extract_pids)
//                traces.erase(pid);
            traces.clear();
        }
        output << "\t# " << traces.size() << " traces is left" << "\n";
        output.flush();
#endif
    }

};

#endif
