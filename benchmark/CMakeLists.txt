cmake_minimum_required(VERSION 3.5)
project(hsmBenchmark)

include_directories(AFTER
    ${CMAKE_CURRENT_SOURCE_DIR}
    header)

add_subdirectory(simple)
add_subdirectory(complex)

