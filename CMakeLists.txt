cmake_minimum_required(VERSION 3.30)
project(blockchain)

set(CMAKE_CXX_STANDARD 23)

add_executable(blockchain main.cpp
        block.cpp
        block.h)
