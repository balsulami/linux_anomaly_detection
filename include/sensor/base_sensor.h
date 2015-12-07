
#pragma once
#include<vector>
#include <fstream>
#include <iterator>
#include <cmath>
#include <exception>
#include<string>
#include"../stream/base_stream.h"
#include"../utils/utils.h"

#include<thread>
#include<chrono>
//#include<unistd.h>

#include"socket/linux_server_socket.h"

std::string FTRACE_PATH = "/sys/kernel/debug/tracing";
std::string SYSCALLS_PATH = "/sys/kernel/debug/tracing/events/raw_syscalls";
std::string SYS_ENTRY = "sys_enter";
std::string SYS_EXIT = "sys_exit";

template<class T>
class BaseSensor{
public:
	BaseSensor(sensor_stream<T> * stream = nullptr) :_stream(stream){
	}

	void config_events(bool entry = true, bool exit = false){
		write_to(SYSCALLS_PATH + "/" + SYS_ENTRY + "/" + "enable", (entry ? "1" : "0"));
		write_to(SYSCALLS_PATH + "/" + SYS_EXIT + "/" + "enable", (exit ? "1" : "0"));
	}

	void add_filter(std::vector<pid_t> & pids){
		std::string filter;
		for (int i = 0; i < pids.size();i++){
			filter.append("common_pid != " + std::to_string(pids[i]));
			if (i < pids.size() - 1)
				filter.append(" && ");
		}
		write_to(SYSCALLS_PATH + "/" + SYS_ENTRY + "/" + "filter", filter);
	}

	bool is_finished(){return finished;}

	~BaseSensor(){
		delete _stream;
	}



protected:
	sensor_stream<T> * _stream;
	mutable bool finished ;

	void set_trace(bool trace) {
		if (trace)
			write_to(FTRACE_PATH + "/tracing_on", "1");
		else {
			write_to(FTRACE_PATH + "/tracing_on", "0");
		}
	}

	void clear_filters(){
		write_to(SYSCALLS_PATH + "/filter", "0");
		write_to(SYSCALLS_PATH + "/" + SYS_ENTRY + "/" + "filter", "0");
		write_to(SYSCALLS_PATH + "/" + SYS_EXIT + "/filter", "0");
	}
};


