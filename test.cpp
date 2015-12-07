#include <iostream>
using namespace std;

#include <Eigen/Dense>
#include <Eigen/Sparse>
using Eigen::MatrixXd;
using Eigen::VectorXd;
using Eigen::SparseMatrix;

#include "detector/ngrams_vector.h"

//#include "detector/svm.h"
#include "detector/sparse_matrix.h"
#include "detector/sparse_svd.h"
#include "detector/anomaly_detector.h"

double var(MatrixXd & mat){
    MatrixXd mean        = mat.colwise().mean();
    VectorXd variance    = (mat.colwise().squaredNorm()/mat.rows()).array() - mean.array()*mean.array();
    return variance.sum();
}


int main(){
//    FastOneClassSVM md;
//    MatrixXd mat = MatrixXd::Random(5,5);
////	cout<<"Mat=\n"<<mat<<"\n";
//    SparseSVD svd(3);
//    auto sm = svd.compute(mat.sparseView());
//    md.train(sm);
//
//    auto d_sm = MatrixXd(sm);
//
//    auto red_var = var(d_sm);
//    auto full_var = var(mat);
//
//    double d=0;
//    for(int i=0;i<red_var.rows();i++){
//        d += red_var(i)/full_var(i);
//        std::cout<<"full="<<d<<"\n";
//    }
//    std::cout<<(red_var.inverse()*full_var).sum()<<"\n";
////    std::cout<<"red="<<red_var<<"\n";
////    std::cout<<"full"<<full_var<<"\n";
//    std::cout<<"full"<<d<<"\n";
//    auto mat1 = MatrixXd::Random(5,5);
//
////	cout<<"Mat2=\n"<<svd.compute(mat1) <<"\n";
//    auto pr = md.predict(svd.compute(mat1));
//    for(int i = 0; i<mat1.rows();i++) {
//        cout << i<<"=" <<pr[i]<< "==> "<<(pr[i] > 0 ? 1:-1)<<"\n";
//    }



    TraceList traces = {{1,2,1,2,5,6,7,8,5,6,7,12,1,2,1,2,5},{6,7,8,5,6,7,12,1,2,1,2,5,6,7,8,5,6},{7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},{12,1,2,1,2,5,6,7,8,5,6,7,12,5,6,7,8,5,6,7,12}};

    AnomalyDetector<SysRecord> detector;
    srand(time(NULL));
    auto & queue = detector.result_queue();

    std::vector<SysRecord> * records = new std::vector<SysRecord>();
    int random_processes = 10 +rand() % 50;
    for (int i = 0; i < random_processes; i++) {
        int random_trace = 7+ rand()% 200;
        for (int j = 0; j< random_trace ; j++) {
            records->emplace_back(i, rand()% 250);
        }
    }
    queue.enqueue(records);

    detector.start();

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::vector<SysRecord> * records = new std::vector<SysRecord>();
        int random_processes = 20 +rand() % 50;
        for (int i = 0; i < random_processes; i++) {
            pid_t random_pid = rand()% 30000;
            int random_trace = 1+ rand()% 200;
            for (int j = 0; j< random_trace ; j++) {
                records->emplace_back(random_pid, rand()% 250);
            }
        }
        cout<<"main ... dispatching "<<random_processes<<" to detector ...\n";
        queue.enqueue(records);
        cout<<"main ... finished dispatching.\n";
    }

//    srand(time(NULL));
//
//    std::vector<SysRecord> records;
//    int random_processes = 50 +rand() % 50;
//    for (int i = 0; i < random_processes; i++) {
//        int random_trace = 7+ rand()% 200;
//        for (int j = 0; j< random_trace ; j++) {
//            records.emplace_back(i, rand()% 250);
//        }
//    }
//    detector.dispatch(records);
//    detector.start();
//
//    while (true) {
//        std::this_thread::sleep_for(std::chrono::seconds(5));
//        std::vector<SysRecord> records;
//        int random_processes = 10 +rand() % 30;
//        for (int i = 0; i < random_processes; i++) {
//            pid_t random_pid = rand()% 30000;
//            int random_trace = 1+ rand()% 200;
//            for (int j = 0; j< random_trace ; j++) {
//                records.emplace_back(random_pid, rand()% 250);
//            }
//        }
//        cout<<"main ... dispatching "<<random_processes<<" to detector ...\n";
//        detector.dispatch(records);
//        cout<<"main ... finished dispatching.\n";
//    }


//    for(Trace & trace :traces){
//        std::cout<<trace<<"\n";
//        auto ngrams = generator.transform(trace);
//        for (auto & ngram:ngrams) {
//            std::cout << ngram << " , ";
//        }
//        std::cout<<"\n";
//    }
//    ngrams_vector vect(2,350, false   );
//    auto sparse = vect.compute_tfidf(traces);
//    SparseSVD svd(1);
//    std::cout<<sparse.shape()<<"\n";
//    sparse.print();
//    auto sm = svd.compute(sparse);
//    std::cout<<"shape=\n"<<sm<<"\n";
//    Eigen::MatrixXd dense(sparse);
//    std::cout<<"shape=\n"<<var(sm)/var(dense)<<"\n";
}