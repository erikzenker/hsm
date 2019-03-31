cmake_minimum_required(VERSION 3.5)
project(hsmTests)

# dependencies
enable_testing()
find_package(GTest MODULE REQUIRED)
if(NOT TARGET hsm::hsm)
    find_package(hsm CONFIG REQUIRED)
endif()

# target defintion
add_executable(hsmIntegrationTests main.cpp)
target_link_libraries(hsmIntegrationTests
        PRIVATE
          hsm::hsm
          GTest::Main
)
add_test(NAME hsm.IntegrationTests
        COMMAND hsmIntegrationTests
)
