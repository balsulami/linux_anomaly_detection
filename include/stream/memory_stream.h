

#pragma once

#include"base_stream.h"
#include<fstream>
#include<vector>
#include<list>
#include<unordered_map>
template<class T>
class memory_stream : public sensor_stream<T>{
public:
	memory_stream(int capacity) {

	}

	virtual void dispatch(std::vector<T> & records){
		std::string json;
		json.reserve(20 * records.size());
		for (auto iter = records.begin(); iter != records.end(); iter++){

			auto it = traces.find(iter->_pid);
			if (it != traces.end()) {
				it->second.push_back(iter->_syscall);
			}
			else{
				std::vector<syscall_t> sys_traces;
				sys_traces.reserve(100);
				sys_traces.push_back(iter->_syscall);
				traces.emplace(iter->_pid,std::move(sys_traces));
			}
		}
	}

	~memory_stream(){
	}

private:
	std::unordered_map<pid_t,std::vector<syscall_t>> traces;
};