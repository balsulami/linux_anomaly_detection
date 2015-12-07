cmake_minimum_required(VERSION 3.1)
project(highperformancelinuxsensors)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

project(highperformancelinuxsensors)

#find_package(Eigen3 REQUIRED)
include_directories(/usr/local/include/eigen3)
include_directories(./include)

set(SOURCE_FILES
    lib/format.cc
    lib/svm.cpp
    main.cpp)

set(TEST_FILES
        lib/format.cc
        lib/svm.cpp
        test.cpp)

# lib
#add_executable(bin/syscall_sensor ${SOURCE_FILES})
add_executable(highperformancelinuxsensors ${SOURCE_FILES} include/detector/sparse_svd.h)
add_executable(testme ${TEST_FILES})

