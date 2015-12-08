//
// Created by root on 12/2/15.
//

#pragma once
#include <Eigen/Dense>
using Eigen::MatrixXd;

typedef int pid_t;
typedef short syscall_t;

typedef std::tuple<short,short> bigram;

typedef std::vector<short> Trace;
typedef std::vector<Trace> TraceList;

typedef std::vector<double> PredictLists;
typedef MatrixXd Matrix;

typedef int index_t;
typedef double value_t;

