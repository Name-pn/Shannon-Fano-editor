cmake_minimum_required(VERSION 3.21)
project(example C)

set(CMAKE_C_STANDARD 23)

add_executable(example main.c libs/compress/compress.c libs/compress/compress.h)
