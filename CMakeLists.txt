cmake_minimum_required(VERSION 3.8)
project(tinytorrent)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost 1.57.0 REQUIRED COMPONENTS system filesystem program_options )
include_directories(${Boost_INCLUDE_DIRS})
include_directories(... ${Boost_INCLUDE_DIRS})

link_directories(... ${Boost_LIBRARY_DIRS})


set(SOURCE_FILES main.cpp src/tcpserver.cpp include/tcpserver.hpp src/tcpconnection.cpp include/tcpconnection.hpp src/tcpclient.cpp include/tcpclient.hpp src/ThreadPool.cpp include/ThreadPool.h)
add_executable(tinytorrent ${SOURCE_FILES})

target_link_libraries(tinytorrent ${Boost_LIBRARIES})