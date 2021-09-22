cmake_minimum_required(VERSION 3.14)
project(cpm_integration CXX)

include(cmake/CPM.cmake)

CPMAddPackage(
    NAME hsm
    GITHUB_REPOSITORY erikzenker/hsm
    VERSION 1.4.7)

add_executable(cpm_integration main.cpp)
target_link_libraries(cpm_integration PRIVATE hsm::hsm)