//
// Created by root on 11/30/15.
//

#ifndef HIGHPERFORMANCELINUXSENSORS_SPARSE_SVD_H
#define HIGHPERFORMANCELINUXSENSORS_SPARSE_SVD_H

#include <Eigen/Dense>
#include <Eigen/Sparse>
using Eigen::MatrixXd;
using Eigen::SparseMatrix;
using Eigen::JacobiSVD;

typedef int index_t;
typedef double value_t;
#include<iostream>

#include "red_svd.h"
typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,Eigen::RowMajor> DMatrix;

class SparseSVD {
public:
    SparseSVD(long components = 2):_components_count(components){}

    template <typename T>
    DMatrix compute(T & sm){
        if (this->_components.size() == 0){
            RedSVD::RedSVD<DMatrix> _svd;
            _svd.compute(sm,_components_count);

            this->_components = _svd.matrixV();
            return _svd.matrixU() * _svd.singularValues().asDiagonal();
        }
        else
            return sm * this->_components;
    }
private:
    long _components_count;
    JacobiSVD<DMatrix>::MatrixVType _components;
};

#endif //HIGHPERFORMANCELINUXSENSORS_SPARSE_SVD_H
