cmake_minimum_required(VERSION 3.23)
project(Huffman C)

set(CMAKE_C_STANDARD 11)

add_executable(Huffman src/main.c src/file_utils.c src/file_utils.h src/huffman_tree.c src/huffman_tree.h)

add_executable(HuffmanV2 v2_src/main_v2.c v2_src/file_utils_v2.c v2_src/file_utils_v2.h v2_src/huffman_tree_v2.c v2_src/huffman_tree_v2.h)

add_executable(HuffmanV3 v3_src/main_v3.c v3_src/file_utils_v3.c v3_src/file_utils_v3.h v3_src/huffman_tree_v3.c v3_src/huffman_tree_v3.h)

add_executable(Test test/test.c test/uint256.c test/uint256.h)

