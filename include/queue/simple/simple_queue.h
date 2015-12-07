//
// Created by root on 11/17/15.
//

#ifndef HIGHPERFORMANCELINUXSENSORS_SIMPLE_QUEUE_H
#define HIGHPERFORMANCELINUXSENSORS_SIMPLE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template <class T>
class SafeQueue
{
public:
    SafeQueue(void)
            : q()
            , m()
            , c()
    {}

    ~SafeQueue(void)
    {}

    void enqueue(T t)
    {
        std::lock_guard<std::mutex> lock(m);
        q.push(t);
        c.notify_all();
    }

    T dequeue(void)
    {
        std::unique_lock<std::mutex> lock(m);
        while(q.empty())
        {
            c.wait(lock);
        }
        T val = q.front();
        q.pop();
        return val;
    }

    void enqueue_bulk(T * messages, size_t size){
        std::lock_guard<std::mutex> lock(m);
        for(int i=0;i<size;i++)
            q.push(messages[i]);
        c.notify_all();
    }

    size_t wait_dequeue_bulk(T * message,size_t capacity){
        std::unique_lock<std::mutex> lock(m);
        size_t size = 0;
        while(q.empty())
        {
            c.wait(lock);
        }
        for(int i=0;i<capacity && !q.empty();i++) {
            message[i] = q.front();
            q.pop();
            size++;
        }
        return size;
    }

    size_t size_approx(){
        return q.size();
    }

private:
    std::queue<T> q;
    mutable std::mutex m;
    std::condition_variable c;
};
#endif //HIGHPERFORMANCELINUXSENSORS_SIMPLE_QUEUE_H
