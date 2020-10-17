cmake_minimum_required(VERSION 3.5)
project(hsm)

# Conan
if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup(TARGETS)
endif()

# Sanatizers
if(${HSM_ASAN})
    set(CMAKE_CXX_FLAGS "-fsanitize=address -fno-omit-frame-pointer")
endif()

if(${HSM_UBSAN})
    set(CMAKE_CXX_FLAGS "-fsanitize=undefined -fno-omit-frame-pointer -fno-sanitize-recover -fsanitize=float-divide-by-zero")
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
if(${HSM_BENCHMARK})
    add_subdirectory(benchmark)
endif()