# cmake -S . -B build
# cmake --build build
cmake_minimum_required(VERSION 3.26)
project (M2_train)

set (CMAKE_CXX_STANDARD 23)

set (CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -march=native")

# pthread is not a CMake package; use the portable Threads module
find_package(Threads REQUIRED)

add_executable(M2s31 mutex_deadlock_avoid.cpp)
target_link_libraries(M2s31 PRIVATE Threads::Threads)

add_executable(M2s32 mutex_deadlock_avoid_sol.cpp)
target_link_libraries(M2s31 PRIVATE Threads::Threads)