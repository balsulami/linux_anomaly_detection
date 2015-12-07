

#pragma  once
#include<vector>


template<typename T>
class sensor_stream{
public:
	sensor_stream() {}
	virtual void dispatch(std::vector<T> & records) = 0;
};