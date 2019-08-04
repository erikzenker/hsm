cmake_minimum_required(VERSION 3.5)
project(hsmTurnstileExample)

# dependencies
if(NOT TARGET hsm::hsm)
    find_package(hsm CONFIG REQUIRED)
endif()

# target defintion
add_executable(
        hsmTurnstileExample 
          main.cpp)

target_link_libraries(hsmTurnstileExample
        PRIVATE
          hsm::hsm)