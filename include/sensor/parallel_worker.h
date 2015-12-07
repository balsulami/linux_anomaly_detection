
#pragma once 

#include"events_measure.h"
#include<thread>
class ParallelWorker {
public:
	ParallelWorker() :_thread(nullptr){}

    void join() {
       if (_thread->joinable())
            _thread->join();
    }

    template <typename...Args>
    void start(Args ... args){
        _thread = new std::thread(std::forward<Args>(args)...);
    }

    ~ParallelWorker() {
        if (_thread != nullptr) {
            _thread->detach();
            delete _thread;
            _thread = nullptr;
        }
    }

protected:
    std::thread *_thread;
};
