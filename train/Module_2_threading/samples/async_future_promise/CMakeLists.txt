# cmake -S . -B build
# cmake --build build
cmake_minimum_required(VERSION 3.26)
project (M2_train)

set (CMAKE_CXX_STANDARD 23)

set (CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -march=native")

# pthread is not a CMake package; use the portable Threads module
find_package(Threads REQUIRED)

add_executable(M2s40 async_task_mgr.cpp)
target_link_libraries(M2s40 PRIVATE Threads::Threads)

add_executable(M2s40_1 async_task_mgr_lambda.cpp)
target_link_libraries(M2s40 PRIVATE Threads::Threads)

add_executable(M2s40_2 async_task_mgr_custom.cpp)
target_link_libraries(M2s40 PRIVATE Threads::Threads)

