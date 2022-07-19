cmake_minimum_required(VERSION 3.22)
project(Huffman C)

set(CMAKE_C_STANDARD 11)

add_executable(Huffman src/main.c src/file_utils.c src/file_utils.h src/huffman_tree.c src/huffman_tree.h)
