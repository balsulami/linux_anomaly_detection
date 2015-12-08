
#pragma once
#include<vector>
#include"stream/base_stream.h"
#include "data_aggregator.h"
#include <fstream>
#include <iterator>
#include <cmath>
#include <exception>
#include "utils/string_ref.h"
#include<string>
#include"base_sensor.h"
#include "queue/message_queue.h"
#include<thread>
#include <iomanip>
#include<sstream>
#include"utils/utils.h"
#include<signal.h>
#include <fstream>
#include "events_bulk.h"
#include "parallel_worker.h"


using std::setw;
using std::left;

void disable_seg_pipe(){
	struct sigaction new_actn, old_actn;
	new_actn.sa_handler = SIG_IGN;
	sigemptyset (&new_actn.sa_mask);
	new_actn.sa_flags = 0;
	sigaction (SIGPIPE, &new_actn, &old_actn);
}

const int READ_BUFFER_SIZE = 1024;
const int BULK_CAPACITY = 512;

template<class T>
class multithreaded_sensor : public BaseSensor<T> {

	typedef message_queue<EventsBulk*> _in_type;
	typedef message_queue<std::vector<T>*> _out_type;

public:
	multithreaded_sensor(int workers_size = 1) : BaseSensor<T>(){
	}

	~multithreaded_sensor(){
	}

	void run(bool logging = false) {
		start();
		pid_t cur_pid = ::get_pid();
		std::vector<pid_t> pids = get_pids(cur_pid);

		this->add_filter(pids);
		this->start_trace();
	}

	void start(){
		this->finished = false;
		disable_seg_pipe();
		_worker.start(multithreaded_sensor<T>::_process_traces, &_stream_queue, &_result_queue);
		_reader.start(multithreaded_sensor<T>::_read_stream, &_stream_queue);
	}

	void stop(){
		this->stop_trace();
		this->clear_filters();
	}

	void join(){
		_reader.join();
		_worker.join();
	}



	_out_type & result_queue(){
		return _result_queue;
	}

private:

	static void _read_stream(_in_type * _stream_queue) {
		std::ios_base::sync_with_stdio(false);
		std::ifstream trace_pipe(FTRACE_PATH + "/" + "trace_pipe");

		char line[READ_BUFFER_SIZE];
		std::string last_line;
		EventsBulk* bulks[BULK_CAPACITY];

		do{
			int i = 0;
			for (; i < BULK_CAPACITY && !trace_pipe.eof(); i++){
				trace_pipe.read(line, READ_BUFFER_SIZE);
				std::string str_line = std::move(last_line);
				str_line.append(line, line + trace_pipe.gcount());
				if (trace_pipe.eof())
					str_line.append("\n");
				std::vector<StringRef> lines = split_string(str_line, "\n", last_line);
				bulks[i] = new EventsBulk(str_line, lines);
			}
			_stream_queue->enqueue_bulk(bulks, i);
		} while (!trace_pipe.eof());
		_stream_queue->enqueue(nullptr);
	}

	static void _process_traces(_in_type * _stream_queue, _out_type * _result_queue){
		EventsBulk* lines[BULK_CAPACITY];
		DataAggregator<T> _data;
		while (true) {
			size_t count = _stream_queue->wait_dequeue_bulk(lines, BULK_CAPACITY);
			for (size_t i = 0; i < count; i++) {
				EventsBulk * bulk = lines[i];
				if (bulk != nullptr) {
					auto &bulk_lines = bulk->lines();
					collect(bulk_lines, _data);
					delete bulk;
				}
				else {
					auto ptr = _data.make_new_pack();
					if (!ptr->empty())
						_result_queue->enqueue(ptr);
					_stream_queue->enqueue(nullptr);
					_result_queue->enqueue(nullptr);
					return;
				}
				if (_data.ready()) {
					auto ptr = _data.make_new_pack();
					_result_queue->enqueue(ptr);
				}
			}
		}
	}

	static void collect(std::vector<StringRef> & lines,DataAggregator<T> & data) {
		for (auto it = lines.begin(); it != lines.end(); it++) {
			if (it->size() > 60) {

				auto begin = it->begin();
				StringRef pid(begin + 17, 5);

				StringRef syscall(begin + 64,3);

				data.append(pid, syscall);
			}
		}
	}

	ParallelWorker _reader;
	ParallelWorker _worker;

	_in_type _stream_queue;
	_out_type _result_queue;

};


