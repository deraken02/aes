cmake_minimum_required(VERSION 3.16)

project(aes)

find_package(OpenSSL REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})

add_executable(aes src/aes.c)

target_include_directories(aes PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${OPENSSL_LIBRARIES}
)

target_link_libraries(aes PRIVATE OpenSSL::SSL OpenSSL::Crypto)
