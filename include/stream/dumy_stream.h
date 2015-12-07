//
// Created by root on 4/15/15.
//


#pragma once

#include"base_stream.h"
#include<vector>
#include<string>
#include <iostream>

template<typename T>
class dumy_stream : public sensor_stream<T>{
public:
    dumy_stream(){
    }

    virtual void dispatch(std::vector<T> & records){
        for (auto iter = records.begin(); iter != records.end(); iter++){
        }
    }

    ~dumy_stream(){
    }
};
