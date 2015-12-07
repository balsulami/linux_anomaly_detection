

#pragma once

#include<vector>
#include<list>

template<typename T>
class DataAggregator {
public:
	DataAggregator(size_t pack_size = 1024){
		__data = new std::vector<T>();
		__data->reserve(pack_size);
		__limit = pack_size;
		
	}

	template<typename... Args>
	void append(Args&&... argsdata) {
		//auto iter = __data[__curr];
//		lock.lock();
		__data->emplace_back(std::forward<Args>(argsdata)...);
//		lock.unlock();
	}

	bool ready(){
		if (__data->size() >= __limit)
			return true;
		else
			return false;
	}

	std::vector<T> * make_new_pack(){
//		lock.lock();
		auto old_data = __data;
		__data = new std::vector<T>();
		__data->reserve(__limit);
//		lock.unlock();
		return old_data;
		//__data.push_back(pack<T>(__max));
	}



	~DataAggregator(){
		/*for (auto iter = __data.begin(); iter != __data.end(); iter++){
			delete iter;
		}*/
	}

private:

	size_t __limit;
	std::vector<T> * __data;

};