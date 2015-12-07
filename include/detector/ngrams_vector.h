//
// Created by root on 12/1/15.
//

#pragma once

#include"sparse_matrix.h"
#include<list>
#include<tuple>
#include "common.h"
#include "ngrams_transformer.h"

class ngrams_vector{
    typedef Eigen::Triplet<double,long> T;
public:
    ngrams_vector(int len = 2,int table_size = 13,bool binary=false):_window(len,table_size){
        _binary = binary;
    }

    SparseMatrixEx compute_tf(TraceList & traces){
        std::vector<T> row_list;
        for (int i = 0; i < traces.size(); i++){
            auto & trace = traces[i];
            auto ngram_trace = _window.transform(trace);
            auto ngram_count = _window.count(ngram_trace);
            for(auto iter = ngram_count.begin();iter!= ngram_count.end();iter++){
                if(_binary)
                    row_list.push_back(T(i,_window.index(iter->first),1));
                else
                    row_list.push_back(T(i,_window.index(iter->first),iter->second));
            }
        }
        SparseMatrixEx csr_matrix(traces.size(),_window.features_size());
        csr_matrix.setFromTriplets(row_list.begin(),row_list.end());
        return csr_matrix;
    }

    SparseMatrixEx compute_tfidf(TraceList & traces){
        std::vector<T> row_list;
        std::vector<std::tuple<int,bigram,double>> syscalls_freq;
        std::map<bigram,long> ngram_procs;
        // first pass to compuate tf & idf
        for (int i = 0; i < traces.size(); i++){
            auto & trace = traces[i];
            auto ngram_trace = _window.transform(trace);
            auto ngram_count = _window.count(ngram_trace);
            for(auto iter = ngram_count.begin();iter!= ngram_count.end();iter++){
                // add ngrams to all processes dictionary
                if(ngram_procs.count(iter->first) > 0)
                    ngram_procs[iter->first]++;
                else
                    ngram_procs[iter->first] = 1;
                // add per doc
                if(_binary)
                    syscalls_freq.emplace_back(i,iter->first,1);
                else
                    syscalls_freq.emplace_back(i,iter->first,iter->second);
            }
        }
        // second iteration for final idf
        std::vector<double> norm(traces.size(),0.0);
        for(auto & syscall: syscalls_freq) {
            double tf = log(std::get<2>(syscall))+1;
            double idf = log(double(traces.size()+1)/(ngram_procs[std::get<1>(syscall)]+1)) +1;
            std::get<2>(syscall) = tf*idf;

            // add up normalization
            norm[std::get<0>(syscall)] += pow(std::get<2>(syscall),2);
        }

        for(auto & syscall: syscalls_freq) {
            // normalization
            auto norm_freq = std::get<2>(syscall) / sqrt(norm[std::get<0>(syscall)]);

            row_list.push_back(T(std::get<0>(syscall), _window.index(std::get<1>(syscall)), norm_freq));
        }
        SparseMatrixEx csr_matrix(traces.size(),_window.features_size());
        csr_matrix.setFromTriplets(row_list.begin(),row_list.end());
        return csr_matrix;
    }

private:
    ngrams_transformer _window;
    bool _binary;
};

