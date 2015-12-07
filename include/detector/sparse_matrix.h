#pragma once
#include "common.h"
#include <Eigen/Dense>
#include <Eigen/Sparse>
using Eigen::MatrixXd;
using Eigen::SparseMatrix;


class SparseMatrixEx : public SparseMatrix<value_t, Eigen::RowMajor, index_t> {
public:
	SparseMatrixEx(index_t rows, index_t cols) :SparseMatrix(rows, cols){};

	std::tuple<int, int> shape(){
		return std::tuple<int, int>(this->rows(), this->cols());
	}

	double ratio(){
		return double(this->nonZeros()) / this->size();
	}
	
	void print(){
		for (int k = 0; k < this->outerSize(); ++k)
		{
			for (SparseMatrixEx::InnerIterator it(*this, k); it; ++it)
				std::cout << "(" << it.row() << "," << it.col() << ")\t" << it.value() << "\n";
		}
	}
};