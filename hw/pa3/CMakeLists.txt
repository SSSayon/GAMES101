cmake_minimum_required(VERSION 3.10)
project(Rasterizer)

find_package(OpenCV REQUIRED)

set(CMAKE_CXX_STANDARD 17)

include_directories(/usr/local/include ./include)
set(SRC_LIST
    src/main.cpp
    src/rasterizer.cpp
    src/Triangle.cpp
    src/Texture.cpp
    src/OBJ_Loader.h
)
add_executable(Rasterizer ${SRC_LIST})
target_link_libraries(Rasterizer ${OpenCV_LIBRARIES})
#target_compile_options(Rasterizer PUBLIC -Wall -Wextra -pedantic)