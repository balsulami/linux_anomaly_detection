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
using Eigen::Matrix;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using Eigen::SparseMatrix;

typedef SparseMatrix<double,Eigen::RowMajor> SMatrix;
typedef Matrix<double,Eigen::Dynamic,Eigen::Dynamic,Eigen::RowMajor> DMatrix;
//#include "detector/anomaly_detector.h"

void print_all(const DMatrix & X) {


    auto sX = X.sparseView();
    for (int k = 0; k < sX.outerSize(); ++k) {
        for (SMatrix::InnerIterator it(sX, k); it; ++it) {
            cout<<it.value()<<"\t";
//            it.row();   // row index
//            it.col();   // col index (here it is equal to k)
//            it.index(); // inner index, here it is equal to it.row()
        }
        cout << "\n";
    }
}
typedef Eigen::Triplet<double> T;

int main(){

    std::vector<std::vector<double>> traces = {{1,2,1,2,5,6,7,8,5},
                        {6,7,8,5,6,7,12,1,2,1,2,5,6,7,8,5,6},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7,6,7,12,1,2,1,2,5},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {7,12,1,2,1,2,1,2,1,2,5,6,7,8,5,6,7},
                        {12,1,2,1,2,5,6,7,8,5,6,7,12,5,6,7,8,5,6,7,12}};


    std::vector<T> tripletList;
    tripletList.reserve(traces.size()*20);
    for(auto out_iter = traces.begin();out_iter != traces.end();out_iter++){
        for(auto inn_iter = out_iter->begin();inn_iter != out_iter->end();inn_iter++){
            tripletList.push_back(T(out_iter-traces.begin(),inn_iter-out_iter->begin(),*inn_iter));
        }
    }
    SMatrix mat(16,30);
    mat.setFromTriplets(tripletList.begin(), tripletList.end());
//    auto sX = mat.sparseView();
    print_all(mat);
    cout<<mat<<'\n';
    cout<<mat.size()<<'\n';
    cout<<mat.nonZeros()<<'\n';
}