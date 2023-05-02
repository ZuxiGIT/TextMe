cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(TextMe VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(
    -Werror
    -Wall
)

add_executable(server
    server.cpp
)

add_executable(client
    client.cpp
)

add_library(crypto
    crypto.cpp
)

add_library(net
    net.cpp
)

target_link_libraries(net
    Ws2_32
    Mswsock
)

target_link_libraries(server
    crypto
    net
)

target_link_libraries(client
    crypto
    net
)
