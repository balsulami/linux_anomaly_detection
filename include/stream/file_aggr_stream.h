

#pragma once

#include"base_stream.h"
#include<fstream>
#include<vector>
#include<list>
#include<unordered_map>
template<class T>
class file_aggr_stream : public sensor_stream<T>{
public:
	file_aggr_stream(const char * name) : _file(name){

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
//			json.append(iter->to_string());
//			json.append("\n");
		}
//		_file.write(json.c_str(), json.size());
//		_file.flush();
	}

	void print_all(){
		for (auto iter = traces.begin();iter != traces.end();iter++){
			_file<<iter->first<<" ==> ";
			for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2++)
				_file<<*iter2<<",";
			_file<<"\n";
		}
		traces.clear();
	}

	~file_aggr_stream(){
		_file.close();
	}

private:
	std::ofstream _file;
	std::unordered_map<pid_t,std::vector<syscall_t>> traces;
};