cmake_minimum_required(VERSION 3.5)
project(hsmBenchmark)

if(${HSM_TEMPLIGHT_PROFILER})
    set(CMAKE_CXX_FLAGS "-Xtemplight -profiler -Xtemplight -ignore-system")
endif()

include_directories(AFTER
    ${CMAKE_CURRENT_SOURCE_DIR}
    header)

add_subdirectory(components)
add_subdirectory(simple)
add_subdirectory(complex)

