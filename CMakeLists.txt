cmake_minimum_required(VERSION 3.28)
project(blockchain)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenSSL REQUIRED)
find_package(MPI REQUIRED)

add_executable(blockchain main.cpp
        block.cpp
        block.h
        blockchain.cpp
        blockchain.h
        utils.cpp
        utils.h
)

target_link_libraries(blockchain OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(blockchain MPI::MPI_CXX)

