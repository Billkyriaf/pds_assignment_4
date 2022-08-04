cmake_minimum_required(VERSION 3.23)
project(Huffman)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O3")

add_executable(Huffman libs/uint128_t.cpp libs/uint256_t.cpp src/serial/main.cpp
        src/serial/huffman_tree.cpp src/serial/file_utils.cpp src/structs.h)

add_executable(HuffmanPthread libs/uint128_t.cpp libs/uint256_t.cpp src/pthread/main_pthread.cpp
        src/pthread/huffman_tree_pthread.cpp src/pthread/file_utils_pthread.cpp src/structs.h)

target_link_libraries(HuffmanPthread pthread)