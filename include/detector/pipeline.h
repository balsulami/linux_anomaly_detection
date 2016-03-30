#pragma once



#include<vector>
#include<map>
#include "sparse_svd.h"
#include "one_class_svm.h"
#include "tf_idf_transformer.h"


using namespace std;
using namespace Eigen;
using Eigen::SparseMatrix;

class Pipeline{
public:
	Pipeline():_transfomer(2,50),_svd(500){

	}

	void train(TraceList & traces) {
		auto ngrams = _transfomer.compute_tfidf(traces);
		auto reduced = _svd.compute(ngrams);
		_svm_model.train(reduced.sparseView());
	}

	PredictLists predict(TraceList & traces){
		auto ngrams = _transfomer.compute_tfidf(traces);
		auto reduced = _svd.compute(ngrams);
		auto svm_results = _svm_model.predict(reduced.sparseView());
		return svm_results;
	}

private:
	TfIdfTransformer _transfomer;
	SparseSVD _svd;
	OneClassSVM _svm_model;
};