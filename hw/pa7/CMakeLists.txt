cmake_minimum_required(VERSION 3.10)
project(RayTracing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -O3 -fopenmp")
# add_compile_options(-std=c++17 -O3)

set(SRC_LIST
    src/main.cpp
    src/Renderer.cpp
    src/Scene.cpp
    src/BVH.cpp
)
add_executable(RayTracing ${SRC_LIST})
