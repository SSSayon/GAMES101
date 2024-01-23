cmake_minimum_required (VERSION 2.8.11)
project (Transformation)

find_package(Eigen3 REQUIRED)
include_directories(EIGEN3_INCLUDE_DIR)

SET(SRC_LIST
    ./src/main.cpp
)

add_compile_options(-std=c++11 -O2 -Wall)

add_executable (Transformation ${SRC_LIST})
