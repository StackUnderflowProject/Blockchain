cmake_minimum_required(VERSION 3.30)
project(blockchain)

set(CMAKE_CXX_STANDARD 23)

find_package(OpenSSL REQUIRED)

add_executable(blockchain main.cpp
        block.cpp
        block.h
        blockchain.cpp
        blockchain.h
        utils.cpp
        utils.h)

target_link_libraries(blockchain OpenSSL::SSL OpenSSL::Crypto)
