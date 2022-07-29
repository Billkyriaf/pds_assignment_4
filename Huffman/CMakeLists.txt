cmake_minimum_required(VERSION 3.22)
project(Huffman C)

set(CMAKE_C_STANDARD 11)

add_executable(Huffman src/main.c src/file_utils.c src/file_utils.h src/huffman_tree.c src/huffman_tree.h)

add_executable(HuffmanV2 v2_src/main_v2.c v2_src/file_utils_v2.c v2_src/file_utils_v2.h v2_src/huffman_tree_v2.c v2_src/huffman_tree_v2.h)

add_executable(Test test/test.c)