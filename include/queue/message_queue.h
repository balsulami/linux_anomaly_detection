//
// Created by root on 9/3/15.
//

#ifndef HIGHPERFORMANCELINUXSENSORS_MESSAGE_QUEUE_H
#define HIGHPERFORMANCELINUXSENSORS_MESSAGE_QUEUE_H
//#include "queue/moodycamel/blockingconcurrentqueue.h"
#include "queue/simple/simple_queue.h"

template<typename T>
class message_queue{
//    typedef moodycamel::BlockingConcurrentQueue<T> _queue_type;
    typedef SafeQueue<T> _queue_type;
public:
    void enqueue(T message){
        _queue.enqueue(message);
    }

    void enqueue_bulk(T * messages, size_t size){
        _queue.enqueue_bulk(messages,size);
    }

    size_t wait_dequeue_bulk(T * message,size_t capacity){
        return _queue.wait_dequeue_bulk(message,capacity);
    }

    size_t size(){
        return _queue.size_approx();
    }

private:
    _queue_type _queue;
};
#endif //HIGHPERFORMANCELINUXSENSORS_MESSAGE_QUEUE_H
