cmake_minimum_required(VERSION 3.23)
project(Huffman)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O3")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

add_executable(Huffman include/uint256/uint128_t.cpp include/uint256/uint256_t.cpp src/serial/main.cpp
        src/huffman.cpp src/structs.h src/timer.cpp src/timer.h src/serial/char_frequency.cpp src/serial/char_frequency.h src/serial/compress.cpp src/serial/compress.h src/serial/decompress.cpp src/serial/decompress.h)

add_executable(HuffmanPthread include/uint256/uint128_t.cpp include/uint256/uint256_t.cpp src/pthread/main_pthread.cpp
        src/file_utils.cpp src/structs.h src/timer.cpp src/timer.h src/pthread/char_frequency_pth.cpp src/pthread/char_frequency_pth.h src/pthread/compress_pth.cpp src/pthread/compress_pth.h src/pthread/decompress_pth.cpp src/pthread/decompress_pth.h)

target_link_libraries(HuffmanPthread pthread)