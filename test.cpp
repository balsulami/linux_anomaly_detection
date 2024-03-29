#include <iostream>
using namespace std;

//#define FRONT_PROCESS
#ifdef DEBUG_LOG
#include<fstream>
std::ofstream traces_out("/root/.clion11/system/cmake/generated/e3f8c42a/e3f8c42a/Release/traces.txt");
std::ofstream debug_out("/root/.clion11/system/cmake/generated/e3f8c42a/e3f8c42a/Release/debug.txt");
#endif

#ifdef FRONT_PROCESS
#define output_stream std::cout
#else
#include<fstream>
std::ofstream _output("/home/bms/Downloads/output.txt");
#define output_stream _output
#endif


#include <Eigen/Dense>
#include <Eigen/Sparse>
using Eigen::MatrixXd;
using Eigen::VectorXd;
using Eigen::SparseMatrix;

#include "detector/anomaly_detector.h"

int main(){

    TraceList traces = {{1,2,1,2,5,6,7,8,5,6,7,12,1,2,1,2,5},
                        {6,7,8,5,6,7,12,1,2,1,2,5,6,7,8,5,6},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {12,1,2,1,2,5,6,7,8,5,6,7,12,5,6,7,8,5,6,7,12}};
    srand(time(nullptr));
    TraceList records;
    int random_processes = 100 +rand() % 50;
    for (int i = 0; i < random_processes; i++) {
        Trace trace;
        int random_trace = 7+ rand()% 200;
        for (int j = 0; j< random_trace ; j++) {
            trace.emplace_back(rand()% 250);
        }
        records.push_back(trace);
    }

    cout<<"records size = "<<records.size()<<endl;

//    Trace t = {1,2,1,2,5,6,7,8,5,6,7,1,2,1,2,5};
//    ngrams_vector bigrams(2,500,false);
//    auto sparse = bigrams.compute_tfidf(traces);
//    sparse.print();
//

    TraceList records2;
    random_processes = 100 +rand() % 50;
    for (int i = 0; i < random_processes; i++) {
        Trace trace;
        int random_trace = 7+ rand()% 200;
        for (int j = 0; j< random_trace ; j++) {
            trace.emplace_back(1 + rand()% 40);
        }
        records2.push_back(trace);
    }

    Pipeline detector;
    detector.train(records2);
    auto preds = detector.predict(records2);

    for(auto & pred : preds) {
        std::cout << pred <<", "<<endl;
    }

}