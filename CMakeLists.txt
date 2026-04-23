cmake_minimum_required(VERSION 3.16)

project(aes)

add_executable(aes src/aes.c)
target_include_directories(aes PRIVATE ${CMAKE_SOURCE_DIR}/include)

