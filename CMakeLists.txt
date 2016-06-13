project(prometheus)
cmake_minimum_required(VERSION 3.0)

SET(CMAKE_CXX_FLAGS "-std=c++11 -Wall")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory("test")
