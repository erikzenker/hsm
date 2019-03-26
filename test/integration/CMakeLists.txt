cmake_minimum_required(VERSION 3.5)
project(hsmTests)

# dependencies
enable_testing()
find_package(GTest MODULE REQUIRED)
if(NOT TARGET hsm::hsm)
    find_package(hsm CONFIG REQUIRED)
endif()

# target defintion
add_executable(hsmTests main.cpp)
target_link_libraries(hsmTests
        PRIVATE
          hsm::hsm
          GTest::Main
)
add_test(NAME hsm.UnitTests
        COMMAND hsmTests
)
