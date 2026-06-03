cmake_minimum_required(VERSION 3.16)

project(aes)

find_package(OpenSSL REQUIRED)

if(NO_OPENSSL)
    add_compile_definitions(NO_OPENSSL)
else()
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

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

if(NOT NO_OPENSSL)
    target_link_libraries(aes PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()
