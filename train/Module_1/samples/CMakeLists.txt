# cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native" ..
# make -j$(nproc)

cmake_minimum_required(VERSION 3.26)
project(M1_train)
set(CMAKE_CXX_STANDARD 23)
# for all levels
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -march=native")

add_executable(M1s1 sample1.cpp)
add_executable(M1s2 sample2.cpp)
add_executable(M1s3 sample3.cpp)
add_executable(M1s4 sample4.cpp)
add_executable(M1s5 dataprocesingAlgo.cpp)
add_executable(M1s6 stlAlgoRange.cpp)

add_executable(M1s7 stlAlgoPractice.cpp)
# specific target
target_compile_options(M1s7 PRIVATE -O3 -march=native)

add_executable(M1s8 stlAlgoPractice_2.cpp)

add_executable(M1s9 stlAlgoPipeline_practice.cpp)
add_executable(M1s10 stlAlgoPipeline_practice_range.cpp)

add_executable(M1s21 stl_Iterator_functor.cpp)
add_executable(M1s22 stl_Iterator_functor_2.cpp)
add_executable(M1s23 stl_Iterator_functor_ranges.cpp)