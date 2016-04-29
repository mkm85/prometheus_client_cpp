project(unittest)

set(src_dir
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

set(src
  ${src_dir}/unit_test.cpp
#  ${src_dir}/test_counter_builder.cpp
  ${src_dir}/collector_registry_test.cpp
#  ${src_dir}/histogram_test.cpp
  ${src_dir}/text_exposition_test.cpp
  )

SET(CMAKE_CXX_FLAGS "-std=c++11")

add_executable(unittest ${src})
