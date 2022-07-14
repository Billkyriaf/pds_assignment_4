cmake_minimum_required(VERSION 3.22)
project(chess_engine C)

set(CMAKE_C_STANDARD 11)

add_executable(chess_engine src/main.c src/objects/board.c src/objects/board.h)
