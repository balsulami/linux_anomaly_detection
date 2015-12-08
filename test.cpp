#include <iostream>
using namespace std;

#define DEBUG_LOG

#include <Eigen/Dense>
#include <Eigen/Sparse>
using Eigen::MatrixXd;
using Eigen::VectorXd;
using Eigen::SparseMatrix;

#include "detector/anomaly_detector.h"

int main(){

    TraceList traces = {{1,2,1,2,5,6,7,8,5,6,7,12,1,2,1,2,5},{6,7,8,5,6,7,12,1,2,1,2,5,6,7,8,5,6},{7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},{12,1,2,1,2,5,6,7,8,5,6,7,12,5,6,7,8,5,6,7,12}};

//    cout<<traces[0]<<"\n";
    anomaly_detector<SysRecord> detector;
//    srand(time(NULL));
//    auto & queue = detector.result_queue();
//
//    std::vector<SysRecord> * records = new std::vector<SysRecord>();
//    int random_processes = 100 +rand() % 50;
//    for (int i = 0; i < random_processes; i++) {
//        int random_trace = 7+ rand()% 200;
//        for (int j = 0; j< random_trace ; j++) {
//            records->emplace_back(i, rand()% 250);
//        }
//    }
//    queue.enqueue(records);

    detector.start();

//    while (true) {
//        std::this_thread::sleep_for(std::chrono::seconds(5));
//        std::vector<SysRecord> * records = new std::vector<SysRecord>();
//        int random_processes = 20 +rand() % 50;
//        for (int i = 0; i < random_processes; i++) {
//            pid_t random_pid = rand()% 30000;
//            int random_trace = 1+ rand()% 200;
//            for (int j = 0; j< random_trace ; j++) {
//                records->emplace_back(random_pid, rand()% 250);
//            }
//        }
//        cout<<"main ... dispatching "<<random_processes<<" to detector ...\n";
//        queue.enqueue(records);
//        cout<<"main ... finished dispatching.\n";
//    }

    cin.get();
    cout<<"stopping the sensor ...\n";
    detector.stop();

#ifdef DEBUG_LOG
    output.close();
    ::traces.close();
#endif
}