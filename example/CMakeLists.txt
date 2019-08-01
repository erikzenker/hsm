cmake_minimum_required(VERSION 3.5)
project(hsmExample)

# dependencies
if(NOT TARGET hsm::hsm)
    find_package(hsm CONFIG REQUIRED)
endif()

# target defintion
add_executable(
        hsmExample 
          main.cpp)

target_link_libraries(hsmExample
        PRIVATE
          hsm::hsm)