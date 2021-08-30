project(unittest)

find_package(Boost REQUIRED COMPONENTS system program_options unit_test_framework)

set(src_dir
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

set(src
  ${src_dir}/unit_test.cpp
  ${src_dir}/collector_registry_test.cpp
  ${src_dir}/custom_collector_test.cpp
  ${src_dir}/text_exposition_test.cpp
  ${src_dir}/text_exposition_parser.cpp
  ${src_dir}/text_exposition_parser_test.cpp
  )

set(performance_test_src
  ${src_dir}/performance_test.cpp
  )



add_executable(unittest "${src}")

target_link_libraries(unittest Boost::unit_test_framework prometheus_cpp)
set_property(TARGET unittest PROPERTY CXX_STANDARD 11)

add_executable(performance_test "${performance_test_src}")

target_link_libraries(performance_test Boost::program_options pthread prometheus_cpp)
set_property(TARGET performance_test PROPERTY CXX_STANDARD 11)
