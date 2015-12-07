//
// Created by root on 9/15/15.
//

#ifndef HIGHPERFORMANCELINUXSENSORS_SYSCALLS_H
#define HIGHPERFORMANCELINUXSENSORS_SYSCALLS_H

#include "utils/string_ref.h"
#include "sensor/data_aggregator.h"

template<typename T>
class Syscalls {
public:
    void collect(std::vector<StringRef> & lines,DataAggregator<T> & data) {
        for (auto it = lines.begin(); it != lines.end(); it++) {
            if (it->size() > 60) {

                auto begin = it->begin();
                StringRef pid(begin + 17, 5);

                StringRef syscall(begin + 64,3);

                data.append(pid, syscall);
            }
        }
    }
};

#endif //HIGHPERFORMANCELINUXSENSORS_SYSCALLS_H
