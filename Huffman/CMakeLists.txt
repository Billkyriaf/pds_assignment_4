cmake_minimum_required(VERSION 3.23)
project(Huffman)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O3")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

add_executable(Huffman include/uint256/uint128_t.cpp include/uint256/uint256_t.cpp src/serial/main.cpp
        src/serial/huffman_tree.cpp src/serial/file_utils.cpp include/structs.h include/timer/timer.cpp include/timer/timer.h)

add_executable(HuffmanPthread include/uint256/uint128_t.cpp include/uint256/uint256_t.cpp src/pthread/main_pthread.cpp
        src/pthread/huffman_pth.cpp src/pthread/file_utils_pth.cpp include/structs.h include/timer/timer.cpp include/timer/timer.h src/pthread/char_frequency_pth.cpp src/pthread/char_frequency_pth.h src/pthread/compress_pth.cpp src/pthread/compress_pth.h src/pthread/decompress_pth.cpp src/pthread/decompress_pth.h)

target_link_libraries(HuffmanPthread pthread)