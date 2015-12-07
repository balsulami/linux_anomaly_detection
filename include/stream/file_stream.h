

#pragma once

#include"base_stream.h"
#include<fstream>

template<class T>
class file_stream : public sensor_stream<T>{
public:
	file_stream(const char * name) : _file(name){

	}

	virtual void dispatch(std::vector<T> & records){
		std::string json;
		json.reserve(20 * records.size());
		for (auto iter = records.begin(); iter != records.end(); iter++){
			json.append(iter->to_string());
			json.append("\n");
		}
		_file.write(json.c_str(), json.size());
		_file.flush();
	}

	~file_stream(){
		_file.close();
	}

private:
	std::ofstream _file;
};