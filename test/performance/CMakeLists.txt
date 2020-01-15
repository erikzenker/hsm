cmake_minimum_required(VERSION 3.5)
project(hsmPerformanceTests)

# dependencies
enable_testing()
if(NOT TARGET hsm::hsm)
    find_package(hsm CONFIG REQUIRED)
endif()

# target defintion
add_executable(hsmPerformanceTests main.cpp)
target_link_libraries(hsmPerformanceTests
        PRIVATE
          hsm::hsm
          gtest_main
)
add_test(NAME hsm.PerformanceTests
        COMMAND hsmPerformanceTests
)
