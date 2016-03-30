//
// Created by root on 9/21/15.
//

#ifndef HIGHPERFORMANCELINUXSENSORS_FAST_ONE_CLASS_SVM_H
#define HIGHPERFORMANCELINUXSENSORS_FAST_ONE_CLASS_SVM_H
#include <Eigen/Dense>
#include <Eigen/Sparse>
using Eigen::MatrixXd;
using Eigen::SparseMatrix;
using Eigen::RowVectorXd;
#include "svm.h"
#include "common.h"
#include <algorithm>


class OneClassSVM{

public:
    OneClassSVM():_model(nullptr){

    }

    ~OneClassSVM(){
      //  _cleanup_model();
    }

    void train(const SparseMatrixXd & sX)
    {
    //    _cleanup_model();
        // initialize param
        _param.svm_type = ONE_CLASS;
        _param.kernel_type = LINEAR;
        _param.degree = 3;
        _param.gamma = 1.0/sX.rows();	// 1/num_features
        _param.coef0 = 0;
        _param.nu = 0.005;
        _param.cache_size = 1000;
        _param.C = 1;
        _param.eps = 1e-3;
        _param.p = 0.1;
        _param.shrinking = 1;
        _param.probability = 0;
        _param.nr_weight = 0;
        _param.weight_label = NULL;
        _param.weight = NULL;

        // init prob
        _prob.l = sX.rows();
//        _prob.y = new double[_prob.l];
        _prob.x = new svm_node *[_prob.l];
        _prob.x_len = new long[_prob.l];

//        std::fill(_prob.y, _prob.y + _prob.l, 1);

        svm_node * _train_space = new svm_node[sX.nonZeros() + sX.rows()];
        int space_idx = 0;
        for (int i = 0; i < sX.outerSize(); ++i) {
            _prob.x[i] = &_train_space[space_idx];
            for (SparseMatrixXd::InnerIterator it(sX, i); it; ++it) {
//                if (it.value() != 0) {
                _train_space[space_idx].index = it.col();
                _train_space[space_idx++].value = it.value();
//                }
            }
            _train_space[space_idx].index = -1;
            _train_space[space_idx++].value = 0;
        }

        this->_model = svm_train(&_prob, &_param);

        this->_model->SV = _clone_sv(_model);

//        delete [] _prob.y;
        delete [] _prob.x;
        delete [] _train_space;
    }

    PredictLists predict(const SparseMatrixXd & mat)
    {
        PredictLists decisions(mat.rows(),0);

        std::vector<svm_node> x_space;
        x_space.reserve(1024);
        for (int i = 0; i < mat.outerSize(); ++i) {
            x_space.clear();
            double retval;
            for (SparseMatrixXd::InnerIterator it(mat, i); it; ++it) {
                x_space.emplace_back(it.col(), it.value());
            }
            x_space.emplace_back(-1, 0);

            svm_predict_values(_model, x_space.data(), &retval);
            decisions[i] = retval;
        }
//        delete [] x_space;
        return decisions;
    }

private:
    svm_parameter _param;
    svm_problem _prob;
    svm_model * _model;


    svm_node ** _clone_sv(const svm_model * model){
        svm_node ** local = new svm_node *[model->l];
        for (size_t i = 0; i<model->l; i++) {
            svm_node *sv = model->SV[i];
            size_t count = 0;
            while (sv->index != -1) {
                count++;
                sv++;
            }
            svm_node * copy_sv = new svm_node[count+1];
            sv = model->SV[i];
            for(size_t j=0;j<count;j++){
                copy_sv[j].index = sv[j].index;
                copy_sv[j].value = sv[j].value;
            }
            local[i] = copy_sv;
        }
        return local;
    }
    void _cleanup_model(){
//        delete [] _train_space;
        svm_free_and_destroy_model(&(this->_model));
    }
};
#endif //HIGHPERFORMANCELINUXSENSORS_FAST_ONE_CLASS_SVM_H
