cmake_minimum_required(VERSION 3.5)
project(hsmPlayerExample)

# dependencies
if(NOT TARGET hsm::hsm)
    find_package(hsm CONFIG REQUIRED)
endif()

# target defintion
add_executable(
        hsmPlayerExample 
          main.cpp)

target_link_libraries(hsmPlayerExample
        PRIVATE
          hsm::hsm)