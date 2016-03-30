//
// Created by root on 12/2/15.
//

#pragma once
#include <Eigen/Dense>
#include <Eigen/Sparse>
using Eigen::MatrixXd;
using Eigen::SparseMatrix;

typedef int pid_t;
typedef short syscall_t;

typedef std::tuple<short,short> bigram;

typedef std::vector<short> Trace;
typedef std::vector<Trace> TraceList;

typedef std::vector<double> PredictLists;
typedef MatrixXd Matrix;

typedef int index_t;
typedef double value_t;

typedef SparseMatrix<value_t, Eigen::RowMajor,index_t> SparseMatrixXd;

template<typename T>
std::tuple<int, int> shape(const T & matrix){
    return std::tuple<int, int>(matrix.rows(), matrix.cols());
}

template<typename T>
double ratio(const T & matrix){
    return double(matrix.nonZeros()) / matrix.size();
}

template<typename T>
void print(const T & matrix){
    for (int k = 0; k < matrix.outerSize(); ++k)
    {
        for (typename T::InnerIterator it(matrix, k); it; ++it)
            std::cout << "(" << it.row() << "," << it.col() << ")\t" << it.value() << "\n";
    }
}