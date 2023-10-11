cmake_minimum_required(VERSION 3.26)
project(big_str_num)

set(CMAKE_CXX_STANDARD 20)

add_executable(big_str_num main.cpp
        embedded-rsa.h
        embedded-rsa.cpp)
