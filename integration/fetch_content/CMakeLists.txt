cmake_minimum_required(VERSION 3.14)
project(fetch_content_integration CXX)

include(FetchContent)

FetchContent_Declare(hsm
  GIT_REPOSITORY https://github.com/erikzenker/hsm.git
  GIT_TAG v1.4.7)

FetchContent_GetProperties(hsm)
if(NOT hsm_POPULATED)
  FetchContent_Populate(hsm)
  add_subdirectory(${hsm_SOURCE_DIR} ${hsm_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_executable(fetch_content_integration main.cpp)
target_link_libraries(fetch_content_integration PRIVATE hsm::hsm)