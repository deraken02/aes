cmake_minimum_required(VERSION 3.16)

project(aes)

find_package(OpenSSL REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})
add_compile_options(-Wextra -Wall -pedantic -g)
set(AES_SOURCES 
    src/cypher.c
    src/decypher.c
    src/aes.c
    src/key.c
)

add_executable(aes ${AES_SOURCES})

target_include_directories(aes PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(aes PRIVATE OpenSSL::SSL OpenSSL::Crypto)
