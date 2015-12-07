
#pragma once
#include<vector>
#include"stream/base_stream.h"
#include "data_aggregator.h"
#include <fstream>
#include <iterator>
#include <cmath>
#include <exception>
#include"events_measure.h"
#include "utils/string_ref.h"
#include<string>
#include"base_sensor.h"
//#include "queue/readerwriterqueue.h"
#include<thread>
#include <iomanip>
#include"events_bulk.h"
#include "parallel_worker.h"
#include "trace/syscall_params.h"

using std::setw;
using std::left;

template<class T>
class single_sensor:public BaseSensor<T> {
	typedef SyscallsParams<T> Tracer;
public:
	single_sensor(sensor_stream<T> * stream, int capacity = 1024):BaseSensor<T>(stream){
	}

	~single_sensor(){
	}

	void start() {
		pid_t cur_pid = get_pid();
		std::vector<pid_t> pids = get_pids(cur_pid);
		this->add_filter(pids);

		this->set_trace(true);

		_read_stream();

		this->set_trace(false);
		this->clear_filters();
	}

	void run(bool whatever){
		start();
	}

	void _read_stream() { //T1 & inQ,T2 & outQ,DataAggregator<T3> & _data,EventsWatch & watch ){
		std::ios_base::sync_with_stdio(false); // "/ccidata/ftrace_logs/trace_pipe-new.txt"
		std::ifstream trace_pipe(FTRACE_PATH + "/" +
											   "trace_pipe"); //"/ccidata/ftrace_logs/trace_pipe-100000.txt"); //"/ccidata/ftrace_logs/trace_pipe-new.txt"); //
		const int size = 1024;
		const int bulk_capacity = 10;

		char line[size];
		std::string last_line;

		do {
			std::vector<EventsBulk *> bulks;
			bulks.reserve(bulk_capacity);
			int i = 0;
			for (; i < bulk_capacity && !trace_pipe.eof(); i++) {
				trace_pipe.read(line, size);
				std::string str_line = std::move(last_line);
				str_line.append(line, line + trace_pipe.gcount());
				if (trace_pipe.eof())
					str_line.append("\n");
				std::vector<StringRef> lines = split_string(str_line, "\n", last_line);
				bulks.push_back(new EventsBulk(str_line, lines));
			}
			_process_traces(bulks);
		} while (!trace_pipe.eof());
	}

	void _process_traces(std::vector<EventsBulk*> &  bulks) { //T1 & inQ,T2 & outQ,DataAggregator<T3> & _data,EventsWatch & watch ){

		for (auto iter = bulks.begin(); iter != bulks.end(); iter++) {
			EventsBulk *bulk = *iter;
			if (bulk != nullptr) {
				auto &bulk_lines = bulk->lines();
				_tracer.collect(bulk_lines,_data);
				delete bulk;
			}
			auto ptr = _data.make_new_pack();
			_send_stream(ptr);
		}
	}

	void _send_stream(std::vector<T> * packet) { //T1 & inQ,T2 & outQ,DataAggregator<T3> & _data,EventsWatch & watch )
        try {
            for (auto iter = packet->begin(); iter != packet->end(); iter++)
                if (iter != nullptr) {
                    this->_stream->dispatch(*iter);
                    delete iter;
                }
            delete packet;
        }
        catch (std::exception &e) {
            std::cout << e.what() << "\n";
        }
    }

private:
	DataAggregator<T> _data;
	Tracer _tracer;
};


