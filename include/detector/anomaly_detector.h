//
// Created by root on 9/21/15.
//

#ifndef HIGHPERFORMANCELINUXSENSORS_ANOMALYDETECTOR_H
#define HIGHPERFORMANCELINUXSENSORS_ANOMALYDETECTOR_H


#include "stream/base_stream.h"
#include "sensor/parallel_worker.h"
#include <algorithm>
#include "pipeline.h"
#include "utils/sys_record.h"
#include <unordered_map>
#include <thread>
#include <mutex>
#include <chrono>
#include "queue/message_queue.h"
#include "sensor/multithreaded_sensor.h"
#include <chrono>

using namespace chrono;

enum class EVENT_TYPE {
    Normal, Anomalous
};




template<typename T>
class AnomalyDetector {
    typedef std::unordered_map<pid_t, std::vector<syscall_t>> PIDTraces;
     typedef message_queue<std::vector<T> *> _out_type;
     typedef void(*_callback)(AnomalyDetector * ptr) ;

    const double threshold = -0.0001;

public:
    AnomalyDetector(int predict_interval = 1, int train_interval = 5, int trace_len = 10)
            : _predict_interval(predict_interval), _train_interval(train_interval), _trace_len(trace_len) {

    }

    void start() {
        linux_sensor.config_events(true, false);
        linux_sensor.run();
        _train();
        _detect();

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
    Pipeline _detector;
    PIDTraces _current_traces;
    multithreaded_sensor<T> linux_sensor;

    void _extract_traces() {
        while(true) {
            try {
                std::vector<T> *events[1024];
                size_t count = result_queue().wait_dequeue_bulk(events, BULK_CAPACITY);
                for (size_t i = 0; i < count; i++) {
                    auto current = events[i];
                    if (current != nullptr) {
                        _append_traces(_current_traces, *current);
                        delete current;
                    }
                    else{
                        // end of traces
                        return;
                    }
                }
            }
            catch (std::exception &e) {
                output_stream << e.what() << endl;
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

     void _train() {
        output_stream << "The detector is in TRAINING MODE."<<endl;
        output_stream<<"\tListening for " << _train_interval << " mins ... "<<endl;
        std::this_thread::sleep_for(std::chrono::minutes(_train_interval));
        linux_sensor.stop_trace();
         // add null to the end of traces
         result_queue().enqueue(nullptr);
         // extract all
         output_stream << "\tExtracting traces for training ... "<<endl;
         _extract_traces();

        TraceList train_traces;
        std::vector<pid_t> train_pids;
        _filter(_current_traces, _trace_len, train_pids, train_traces);
        // debug message
        output_stream << "\tTraining the detector ...." << endl;
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        _detector.train(train_traces);
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto duration = duration_cast<minutes>(t2 - t1).count();
        output_stream << "\t****** " << duration << " mins to train the detector\n";
    }

    void _detect() {
        linux_sensor.start_trace();
        output_stream <<endl<< "The detector is in DETECTION MODE."<< endl;
        int iter = 1;
        while(true) {
            output_stream << "\tThe detector is listening for " << this->_predict_interval << " mins ... "<<endl;
            std::this_thread::sleep_for(std::chrono::minutes(this->_predict_interval));
            result_queue().enqueue(nullptr);
            _extract_traces();
            output_stream << "\t["<<iter<<"] Extracting traces for predictions ... " << endl;
            TraceList pred_traces;
            std::vector<pid_t> pred_pids;
            _filter(this->_current_traces, this->_trace_len, pred_pids, pred_traces);
            PredictLists predictions = this->_detector.predict(pred_traces);
            output_stream << "\tThe detection results are :"<< endl;
            long normal_pids_size = std::count_if(predictions.begin(), predictions.end(),
                                                  [](double pred) { return pred > -0.0001; });
            output_stream << "\tnormal pids=" << normal_pids_size << endl;
            output_stream << "\tanomlous pids=" << (predictions.size() - normal_pids_size) << endl<<endl;
            output_stream << "\tpredictions=";
            for(int i=0;i<predictions.size();i++){
                output_stream << " ,"<<predictions[i];
            }
            output_stream<<endl;
            ++iter;
        }
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
    }

};

#endif
