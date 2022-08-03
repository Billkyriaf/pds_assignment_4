cmake_minimum_required(VERSION 3.23)
project(Huffman_cpp)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O3")

add_executable(HuffmanV4 libs/uint128_t.cpp libs/uint256_t.cpp src/main.cpp src/huffman_tree.cpp src/file_utils.cpp)