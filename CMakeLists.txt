project(prometheus)
cmake_minimum_required(VERSION 3.0)

add_library(prometheus_cpp INTERFACE)
target_include_directories(prometheus_cpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory("test")
