cmake_minimum_required(VERSION 3.23)
project(Huffman)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O3 -g")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

add_executable(Huffman
        include/uint256/uint128_t.cpp
        include/uint256/uint256_t.cpp
        src/sequential/main.cpp
        src/file_utils.cpp
        src/huffman.cpp
        src/structs.h
        src/timer.cpp
        src/sequential/char_frequency.cpp
        src/sequential/compress.cpp
        src/sequential/decompress.cpp
)

add_executable(HuffmanPthread
        include/uint256/uint128_t.cpp
        include/uint256/uint256_t.cpp
        src/pthread/main_pthread.cpp
        src/file_utils.cpp
        src/huffman.cpp
        src/structs.h
        src/timer.cpp
        src/pthread/char_frequency_pth.cpp
        src/pthread/compress_pth.cpp
        src/pthread/decompress_pth.cpp
)
target_link_libraries(HuffmanPthread pthread)

add_executable(HuffmanCilk
        include/uint256/uint128_t.cpp
        include/uint256/uint256_t.cpp
        src/cilk/main_cilk.cpp
        src/file_utils.cpp
        src/huffman.cpp
        src/structs.h
        src/timer.cpp
        src/cilk/char_frequency_cilk.cpp
        src/cilk/compress_cilk.cpp
        src/cilk/decompress_cilk.cpp
)
target_link_libraries(HuffmanCilk -fopencilk)