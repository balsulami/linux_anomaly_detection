//
// Created by root on 12/1/15.
//

#pragma once

#include"sparse_matrix.h"
#include<list>
#include<tuple>
#include "common.h"

std::ostream & operator<<(std::ostream & out,const bigram & ngrams){
    out<<"("<<std::get<0>(ngrams)<<","<<std::get<1>(ngrams)<<")";
    return out;
}

std::ostream & operator<<(std::ostream & out,const Trace & trace){
    std::cout<<"[";
    for(auto & syscall: trace){
        out<<syscall<<",";
    }
    std::cout<<"]";
    return out;
}

namespace std
{
    template<> struct less<bigram> {
        bool operator()(const bigram &lhs, const bigram &rhs) const {
            return std::get<0>(lhs) < std::get<0>(rhs)
            || (!(std::get<0>(rhs) < std::get<0>(lhs)) && std::get<1>(lhs) < std::get<1>(rhs));
        }
    };
}


class ngrams_transformer{
public:
    ngrams_transformer(int len = 2,int table_size = 13):_len(len),_table_size(table_size),_voca_size(std::pow(table_size,len)){

    }

    long index(const bigram & ngrams){
        return std::pow(_table_size,1) * std::get<0>(ngrams) +
                std::pow(_table_size,0) * std::get<1>(ngrams);
    }

    long features_size(){
        return _voca_size;
    }

    std::vector<bigram> transform(Trace & trace){
        std::vector<bigram> ngrams;
        ngrams.reserve(trace.size()-1);
        for(auto iter=trace.begin();iter != trace.end();iter++){
            if(iter+1<trace.end())
                ngrams.push_back(std::make_tuple(*iter,*(iter+1)));
        }
        return ngrams;
    }

    std::map<bigram,long> count(std::vector<bigram> & ngrams){
        // new dictonary for the row
        std::map<bigram,long> row_dict;
        for(auto ngrams_iter=ngrams.begin();ngrams_iter != ngrams.end();ngrams_iter++){
            if(row_dict.count(*ngrams_iter) > 0)
                row_dict[*ngrams_iter]++;
            else
                row_dict[*ngrams_iter] = 1;
        }
        return row_dict;
    }

private:
    long _table_size;
    long _voca_size;
    int _len;
};