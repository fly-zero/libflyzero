cmake_minimum_required(VERSION 3.10)

project(flyzero VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(flyzero STATIC
    src/event_base.cpp
    src/hash.cpp
    src/hex.cpp
    src/ipv4_addr.cpp
    src/ipv6_addr.cpp
    src/memory.cpp
    src/mempool.cpp
    src/task_queue_thread.cpp
    src/tcp_connection.cpp
    src/tcp_server.cpp
    src/utility.cpp
    src/circular_buffer.c)