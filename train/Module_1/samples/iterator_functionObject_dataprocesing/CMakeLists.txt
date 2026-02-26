# cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native" ..
# make -j$(nproc)

cmake_minimum_required(VERSION 3.26)
project(M1_train)
set(CMAKE_CXX_STANDARD 23)
# for all levels
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -march=native")

add_executable(M1s1e1 iterator_processing.cpp)
