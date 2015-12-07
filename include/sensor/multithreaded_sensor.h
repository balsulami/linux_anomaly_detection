
#pragma once
#include<vector>
#include"stream/base_stream.h"
#include "data_aggregator.h"
#include <fstream>
#include <iterator>
#include <cmath>
#include <exception>
#include"sensor/events_measure.h"
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
#include "trace/syscalls.h"
#include "stream/file_aggr_stream.h"
#include "events_bulk.h"

const int READ_BUFFER_SIZE = 1024;
const int BULK_CAPACITY = 512;

using std::setw;
using std::left;

void disable_seg_pipe(){
	struct sigaction new_actn, old_actn;
	new_actn.sa_handler = SIG_IGN;
	sigemptyset (&new_actn.sa_mask);
	new_actn.sa_flags = 0;
	sigaction (SIGPIPE, &new_actn, &old_actn);
}

template<class T>
class multithreaded_sensor : public BaseSensor<T> {

	typedef Syscalls<T> Tracer;
	typedef message_queue<EventsBulk*> _in_type;
	typedef message_queue<std::vector<T>*> _out_type;


public:
	multithreaded_sensor(int workers_size = 1) : BaseSensor<T>(){
	}

	virtual std::string get_status() {
		std::stringstream status;
		std::locale comma_locale(std::locale(), new comma_numpunct());
		status.imbue(comma_locale);
		int len = 25;
		int len_events = 15;
		status << setw(len) << left << "Read Events " << "= " << std::setprecision(2) << std::fixed << setw(len_events) << _reader_watch.now() << " events/sec\n";
		status << setw(len) << left << "Process Events " << "= " << std::setprecision(2) << std::fixed << setw(len_events) << _worker_watch.now() << " events/sec\n";
		status << setw(len) << left << "Stream Queue size " << "= " << setw(len_events) << _stream_queue.size() << "\n";
		status << setw(len) << left << "Result Queue size " << "= " << setw(len_events) << _result_queue.size() << "\n" << "\n";
		std::cout<<status<<"\n";
		return status.str();
	}

	virtual std::string get_summary() {
		std::stringstream status;
		std::locale comma_locale(std::locale(), new comma_numpunct());
		status.imbue(comma_locale);
		int len = 25;
		int len_events = 15;
		status << setw(len) << left << "All Events " << "= " << std::setprecision(2) << std::fixed << setw(len_events) << _reader_watch.all_events() << " events\n";
		status << setw(len) << left << "Read Events " << "= " << std::setprecision(2) << std::fixed << setw(len_events) << _reader_watch.total() << " events/sec\n";
		status << setw(len) << left << "Process Events " << "= " << std::setprecision(2) << std::fixed << setw(len_events) << _worker_watch.total() << " events/sec\n";
		return status.str();
	}

	~multithreaded_sensor(){
	}

	void run(bool logging = false) {
		start();
		pid_t cur_pid = ::get_pid();
		std::vector<pid_t> pids = get_pids(cur_pid);

		this->add_filter(pids);
		this->set_trace(true);
		if (!logging)
			join();
		else {
			do {
				try {
					while (!_socket.listen())
						sleep_sec(1);
					while (!this->is_finished()) {
						sleep_sec(1);
						std::string status = get_status();
						int ret = _socket.send(status);
						if (ret == -1) {
							_socket.close();
							break;
						}
					}
				} catch (std::exception e) {
					// TODO add log file
				}
			} while (!this->is_finished());
			_socket.send(get_summary());
			_socket.close();
		}
		this->set_trace(false);
		this->clear_filters();
		stop();
	}

	void start(){
		this->finished = false;
		disable_seg_pipe();
//		_sender.start(multithreaded_sensor<T>::_send_stream, &_result_queue, &_sender_watch, this->_stream, &this->finished);
		_worker.start(multithreaded_sensor<T>::_process_traces, &_stream_queue, &_result_queue, &_data, &_tracer,&_worker_watch);
		_reader.start(multithreaded_sensor<T>::_read_stream, &_stream_queue, &_reader_watch);
	}

	void stop(){
	}

	void join(){
		_reader.join();
		_worker.join();
	}

	static void _read_stream(_in_type * _stream_queue, EventsWatch * _watch) { //T1 & inQ,T2 & outQ,DataAggregator<T3> & _data,EventsWatch & watch ){
		std::ios_base::sync_with_stdio(false); // "/ccidata/ftrace_logs/trace_pipe-new.txt"
		std::ifstream trace_pipe(FTRACE_PATH + "/" + "trace_pipe"); //"
		_watch->start();

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
				_watch->add(lines.size());
				bulks[i] = new EventsBulk(str_line, lines);
			}
			_stream_queue->enqueue_bulk(bulks, i);
		} while (!trace_pipe.eof());
		_stream_queue->enqueue(nullptr);
		_watch->stop();
	}

	static void _process_traces(_in_type * _stream_queue, _out_type * _result_queue, DataAggregator<T> * _data, Tracer * _tracer,EventsWatch * _watch){ //T1 & inQ,T2 & outQ,DataAggregator<T3> & _data,EventsWatch & watch ){
		_watch->start();
		EventsBulk* lines[BULK_CAPACITY];
		while (true) {
			size_t count = _stream_queue->wait_dequeue_bulk(lines, BULK_CAPACITY);
			for (size_t i = 0; i < count; i++) {
				EventsBulk * bulk = lines[i];
				if (bulk != nullptr) {
					auto &bulk_lines = bulk->lines();
					_tracer->collect(bulk_lines, *_data);
					delete bulk;
				}
				else {
					auto ptr = _data->make_new_pack();
					if (!ptr->empty())
						_result_queue->enqueue(ptr);
					_stream_queue->enqueue(nullptr);
					_result_queue->enqueue(nullptr);
					_watch->stop();
					return;
				}
				if (_data->ready()) {
					auto ptr = _data->make_new_pack();
					_result_queue->enqueue(ptr);
				}
			}
		}
	}

	_out_type & result_queue(){
		return _result_queue;
	}



private:
	ParallelWorker _reader;
	ParallelWorker _worker;

	EventsWatch _reader_watch, _worker_watch;

	_in_type _stream_queue;
	_out_type _result_queue;

	linux_server_socket _socket;

	DataAggregator<T> _data;
	Tracer _tracer;

};


