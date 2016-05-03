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

set(performance_test_src
  ${src_dir}/performance_test.cpp
  )

SET(CMAKE_CXX_FLAGS "-std=c++11")

add_executable(unittest ${src})

add_executable(performance_test ${performance_test_src})

target_link_libraries(performance_test boost_program_options pthread)