

#pragma once

#include"base_stream.h"
#include<vector>
#include<string>
#include <iostream>

template<typename T>
class console_stream : public sensor_stream<T>{
public:
	console_stream(){
	}

	virtual void dispatch(std::vector<T> & records){
		/*std::string buffer;
		for (auto iter = records.get_data()->records->begin(); iter != records.get_data()->records->end(); iter++)
			std::cout<<"syscall("<<iter->to_string()<<")\n";*/
		for (auto iter = records.begin(); iter != records.end(); iter++){
			std::cout << iter->to_string()<<"\n";
		}		
		/*std::string json;
		json.reserve(20 * current->size());
		json.append(iter->to_string());*/

	}

	~console_stream(){
	}
};