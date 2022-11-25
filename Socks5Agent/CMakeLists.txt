cmake_minimum_required(VERSION 3.20)
project(Socks5Agent C)

set(CMAKE_C_STANDARD 11)

add_executable(Socks5Agent main.c)

target_link_libraries(Socks5Agent ws2_32)

# Static linking libgcc and winpthread.
if(NOT MSVC)
    if (${CMAKE_BUILD_TYPE} MATCHES "Release")
        target_link_options(Socks5Agent PRIVATE -static-libgcc -static -lwinpthread -dynamic -pthread -O3 -s)
    elseif(${CMAKE_BUILD_TYPE} MATCHES "Debug")
        target_link_options(Socks5Agent PRIVATE -static-libgcc -static -lwinpthread -dynamic -pthread -O0)
    endif()
endif()
