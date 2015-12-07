#pragma once



#include<vector>
#include<map>
#include"sparse_matrix.h"
#include "sparse_svd.h"
#include "one_class_svm.h"
#include "ngrams_transformer.h"


using namespace std;
using namespace Eigen;


class pipeline_estimator{
public:
	pipeline_estimator():_transfomer(2,350),_svd(100){

	}

	void train(TraceList & traces){
		auto ngrams_sparse = _transfomer.compute_tfidf(traces);
		auto reduced_ngras = _svd.compute(ngrams_sparse);
		_svm_model.train(reduced_ngras);
	}

	PredictLists predict(TraceList & traces){
		auto ngrams_sparse = _transfomer.compute_tfidf(traces);
		auto reduced_ngras = _svd.compute(ngrams_sparse);
		auto svm_results = _svm_model.predict(reduced_ngras);
		return svm_results;
	}

private:
	ngrams_vector _transfomer;
	SparseSVD _svd;
	one_class_svm _svm_model;
};