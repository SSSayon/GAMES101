cmake_minimum_required(VERSION 3.10)
project(RayTracing)

set(CMAKE_CXX_STANDARD 17)

set(SRC_LIST
    src/main.cpp
    src/Renderer.cpp
    src/Scene.cpp
    src/BVH.cpp
)
add_executable(RayTracing ${SRC_LIST})
