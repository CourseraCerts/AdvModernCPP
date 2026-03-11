# cmake -S . -B build
# cmake --build build
cmake_minimum_required(VERSION 3.26)
project (M2_train)

set (CMAKE_CXX_STANDARD 23)

set (CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -march=native")

# pthread is not a CMake package; use the portable Threads module
find_package(Threads REQUIRED)

add_executable(M2s10 hw.cpp)

add_executable(M2s11 thread_sync.cpp)
target_link_libraries(M2s11 PRIVATE Threads::Threads)

add_executable(M2s12 deadlock_prevention.cpp)
target_link_libraries(M2s11 PRIVATE Threads::Threads)

add_executable(M2s13 producer_consumer_pq.cpp)
target_link_libraries(M2s11 PRIVATE Threads::Threads)