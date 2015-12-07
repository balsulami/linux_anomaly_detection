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
enum EVENT_TYPE {
    Normal, Anomalous
};


template<typename T>
class AnomalyDetector {
    typedef std::unordered_map<pid_t, std::vector<syscall_t>> PIDTraces;
    typedef message_queue<std::vector<T> *> _out_type;

public:
    AnomalyDetector(int predict_interval = 30, int train_interval = 1, int trace_len = 10)
            : _predict_interval(predict_interval), _train_interval(train_interval), _trace_len(trace_len) {

    }

    void start() {
//        linux_sensor.config_events(true, false);
//        linux_sensor.run(false);

        cout << "The detector is listening for " << _train_interval << " mins ... \n";
        //std::this_thread::sleep_for(std::chrono::seconds(20)); //minutes(_train_interval));
//        auto traces = _extract_traces(_current_traces, _trace_len);
        TraceList train_traces;
        std::vector<pid_t> train_pids;
        AnomalyDetector::_pull_traces1(this);
        cout << "Extracting traces for training ... \n";
        _filter(_current_traces, _trace_len, train_pids, train_traces);
        // debug message
        cout << "training the detector ...." << "\n";
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        _detector.train(train_traces);
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto duration = duration_cast<seconds>(t2 - t1).count();
        cout << "****** " << duration << " seconds to train the detector\n";
        _worker.start(AnomalyDetector::_pull_traces, this);
//        cout << "The detector is filling up\n";
//        fillup_traces(this);
        cout << "The detector is running\n";
//        _start_detect(this);
////        _worker.start(AnomalyDetector::_start_train, this);
    }

    _out_type & result_queue(){
        return linux_sensor.result_queue();
    }

private:
    int _predict_interval, _train_interval;
    int _trace_len;
    pipeline_estimator _detector;
    PIDTraces _current_traces;
    ParallelWorker _worker;
    multithreaded_sensor<T> linux_sensor;

    void static _pull_traces1(AnomalyDetector *ptr) { //T1 & inQ,T2 & outQ,DataAggregator<T3> & _data,EventsWatch & watch ){
        while (true) {
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
                break;
            }
            catch (std::exception &e) {
                std::cout << e.what() << "\n";
            }
        }
    }

    void static _pull_traces(AnomalyDetector *ptr) { //T1 & inQ,T2 & outQ,DataAggregator<T3> & _data,EventsWatch & watch ){
        while (true) {
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
                _detect(ptr);
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
                sys_traces.reserve(200);
                sys_traces.push_back(iter->_syscall);
                _current_traces.emplace(iter->_pid, std::move(sys_traces));
            }
        }
    }

    static void _detect(AnomalyDetector *ptr) {
        cout << "Extracting traces for predictions ... \n";
        TraceList pred_traces;
        std::vector<pid_t> pred_pids;
        _filter(ptr->_current_traces, ptr->_trace_len, pred_pids, pred_traces);
        PredictLists predictions = ptr->_detector.predict(pred_traces);
        cout << "The detection results are :\n";
        long normal_pids = std::count_if(predictions.begin(), predictions.end(),
                                         [](double pred) { return pred > 0.0; });
        cout << "\tnormal pids=" << normal_pids << "\n";
        cout << "\tanomlous pids=" << (predictions.size() - normal_pids) << "\n";
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
        cout << "\t# " << extract_pids.size() << " traces is used" << "\n";
        if (extract_pids.size() > 0) {
            cout << "\tMaximum trace length = " << max << "\n";
            cout << "\tMinimum trace length = " << min << "\n";
            cout << "\tAverage trace length = " << sum / extract_pids.size() << "\n";
            for (auto &pid: extract_pids)
                traces.erase(pid);
        }
        cout << "\t# " << traces.size() << " traces is left" << "\n";
    }

};

#endif
