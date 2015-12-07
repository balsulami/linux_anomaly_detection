//
// Created by root on 9/21/15.
//

#ifndef HIGHPERFORMANCELINUXSENSORS_FAST_ONE_CLASS_SVM_H
#define HIGHPERFORMANCELINUXSENSORS_FAST_ONE_CLASS_SVM_H
#include <Eigen/Dense>
#include <Eigen/Sparse>
using Eigen::MatrixXd;
using Eigen::SparseMatrix;

#include "svm.h"

#include <algorithm>


class one_class_svm{

public:
    one_class_svm():_model(nullptr),_train_space(nullptr){

    }

    ~one_class_svm(){
        _cleanup_model();
    }

    void train(const Matrix & mat)
    {
        _cleanup_model();
        // initialize param
        _param.svm_type = ONE_CLASS;
        _param.kernel_type = LINEAR;
        _param.degree = 3;
        _param.gamma = 0;	// 1/num_features
        _param.coef0 = 0;
        _param.nu = 0.005;
        _param.cache_size = 100;
        _param.C = 1;
        _param.eps = 1e-3;
        _param.p = 0.1;
        _param.shrinking = 1;
        _param.probability = 0;
        _param.nr_weight = 0;
        _param.weight_label = NULL;
        _param.weight = NULL;

        // init prob
        _prob.l = mat.rows();
        _prob.y = new double[_prob.l];
        _prob.x = new svm_node *[_prob.l];

        std::fill(_prob.y, _prob.y + _prob.l, 1);

        _train_space = new svm_node[mat.rows() * mat.cols()+mat.rows()];
        for (size_t i = 0, outer_index = 0; i < mat.rows(); i++)
        {
            _prob.x[i] = &_train_space[outer_index];
            for (size_t j = 0; j<mat.cols(); j++)
            {
                _train_space[outer_index].index = j;
                _train_space[outer_index++].value = mat(i, j);
            }
            _train_space[outer_index++].index = -1;
        }

        this->_model = svm_train(&_prob, &_param);

        delete [] _prob.y;
        delete [] _prob.x;
        delete [] _train_space;
    }

    PredictLists predict(const Matrix & mat)
    {
        svm_node * x_space = new svm_node[mat.cols() + 1];
        PredictLists decisions(mat.rows(),0);

        for (size_t i = 0; i < mat.rows(); i++)
        {
            double retval;
            for (size_t j = 0; j<mat.cols(); j++)
            {
                x_space[j].index = j;
                x_space[j].value = mat(i, j);
            }
            x_space[mat.cols()].index = -1;

            svm_predict_values(_model, x_space, &retval);
            decisions[i] = retval;
        }

        delete [] x_space;
        return decisions;
    }

private:
    svm_parameter _param;
    svm_problem _prob;
    svm_model * _model;
    svm_node * _train_space;

    void _cleanup_model(){
//        delete [] _train_space;
        svm_free_and_destroy_model(&(this->_model));
    }
};
#endif //HIGHPERFORMANCELINUXSENSORS_FAST_ONE_CLASS_SVM_H
