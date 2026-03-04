# cmake -S . -B build
# cmake --build build
cmake_minimum_required(VERSION 3.26)
project (M2_train)

set (CMAKE_CXX_STANDARD 23)

set (CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -march=native")

# pthread is not a CMake package; use the portable Threads module
find_package(Threads REQUIRED)

# link the imported target provided by Threads
add_executable(M2s1 thread_mgmt.cpp)
target_link_libraries(M2s1 PRIVATE Threads::Threads)

add_executable(M2s2 thread_pool.cpp)
target_link_libraries(M2s1 PRIVATE Threads::Threads)

add_executable(M2s2__1 thread_pool_latest.cpp)
target_link_libraries(M2s1 PRIVATE Threads::Threads)

add_executable(M2s2__2 thread_pool_latest_older.cpp)
target_link_libraries(M2s1 PRIVATE Threads::Threads)

add_executable(M2s3 condition_variables.cpp)
target_link_libraries(M2s1 PRIVATE Threads::Threads)

add_executable(M2s4 thread_lifecycle.cpp)
target_link_libraries(M2s1 PRIVATE Threads::Threads)