
#pragma once

#include"base_stream.h"
#include<vector>
#include<string>
#include <chrono>
#include<algorithm>
#include"../utils/functions.h"
#include"../utils/format.h"
#include<cstring>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include"socket/win_socket.h"
#else
#include"socket/linux_socket.h"
#endif

using namespace std::chrono;



template<typename T>
struct sensor_packet_json
{
public:
	sensor_packet_json(std::string meta, std::vector<T> & records) :__records(records), _meta(meta){
	}

	std::string to_json(){
		fmt::MemoryWriter w;
		w.clear();
		w << "{ \"sensor_info\": \"" << _meta <<"\",";
		w << "\"timestamp\": \"" << duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() << "\",";
		w << "\"syscalls\":[";
		for (auto iter = __records.begin(); iter < __records.end(); iter++){
			w << "(" << iter->to_string() << ")";
			if (iter != __records.end() - 1)
				w << ",";
		}
		w << "]}" << '\0';
		return w.data();
	}




private:
	std::string _meta;
	std::vector<T> & __records;
};

template<typename T>
class socket_stream : public sensor_stream<T>{
public:
	socket_stream(const std::string & sensor_info, const char * address, const int port) : _sensor_info(sensor_info), _socket(address, port){
		_socket.connect();
	}

	virtual void dispatch(std::vector<T> & records){
		sensor_packet_json<T> json_packet(_sensor_info, records);
		std::string json = json_packet.to_json();
		std::string str(4, ' ');
		str.append(base64_encode((unsigned char*)json.c_str(), json.length()));
		int n = str.length()-4;

		uint8_t myByteArray[sizeof(n)];
		int32_t bigEndianValue = htonl(n);  // convert the value to big-endian for cross-platform compatibility
		memcpy(&myByteArray[0], &bigEndianValue, sizeof(bigEndianValue));

		str[0] = myByteArray[0];
		str[1] = myByteArray[1];
		str[2] = myByteArray[2];
		str[3] = myByteArray[3];
		_socket.send(str);
	}

	~socket_stream(){
	}

private:
	std::string _sensor_info;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	win_socket _socket;
#else
	linux_socket _socket;
#endif
	
};