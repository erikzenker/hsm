cmake_minimum_required(VERSION 3.5)
project(hsm)

if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup(TARGETS)
endif()

# dependencies
find_package(Boost 1.72 REQUIRED)

add_subdirectory(vendor/googletest)

# add project code
add_subdirectory(src)

# add unit tests
enable_testing()
add_subdirectory(test)

# add example
add_subdirectory(example)

# add benchmark
add_subdirectory(benchmark)