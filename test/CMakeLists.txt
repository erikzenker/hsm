cmake_minimum_required(VERSION 3.5)
project(hsm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -Werror -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Werror -Wall -Wextra -Wpedantic -g3 -O0")

if(${HSM_CLANG_COVERAGE})
    set(CMAKE_CXX_FLAGS "-fprofile-instr-generate -fcoverage-mapping")
endif()

if(${HSM_GCC_COVERAGE})
    set(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
endif()

add_subdirectory(integration)
add_subdirectory(performance)
add_subdirectory(unit)


