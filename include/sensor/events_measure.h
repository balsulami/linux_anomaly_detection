


#pragma once


#include <chrono>
#include<iostream>
#include <iomanip>
#include <atomic>
#include "utils/comma_numpunct.h"

using namespace std::chrono;

class EventsWatch{
public:
	EventsWatch():events(0),finished(0),total_events(0){}
	void start(){
		start_all = start_p = high_resolution_clock::now();		 
	}

	void add(int events){
		this->events+=events;
		this->total_events += events;
	}

	void stop(){
		stop_p = high_resolution_clock::now();
		finished = true;
	}

	void reset(){
		events = 0;
		start_p = high_resolution_clock::now();
	}

	bool is_finished(){ return finished; }

	size_t now(){
		auto sec = duration_cast<microseconds>(high_resolution_clock::now() - start_p).count() / 1000000;
		if (sec == 0){
			//std::cout << "sec is zero";
			return 0;
		}
		size_t events_ratio = events / sec;
		reset();
		return events_ratio;
	}

	size_t total(){
		auto sec = duration_cast<microseconds>(stop_p - start_all).count() / 1000000;
		if (sec == 0){
			//std::cout << "sec is zero";
			return 0;
		}
		return total_events / sec;
	}

	microseconds microsec(){
		return duration_cast<microseconds>(stop_p - start_all);	
	}


	size_t all_events(){ return total_events; }

private:
	high_resolution_clock::time_point start_p,start_all, stop_p;
	size_t events;//std::atomic<
	size_t total_events;
	bool finished;
};