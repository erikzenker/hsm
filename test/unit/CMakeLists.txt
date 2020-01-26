cmake_minimum_required(VERSION 3.5)
project(hsmTests)

# dependencies
enable_testing()
if(NOT TARGET hsm::hsm)
    find_package(hsm CONFIG REQUIRED)
endif()

# target defintion
add_executable(
        hsmUnitTests 
          main.cpp 
          collect_states_tests.cpp 
          collect_events_tests.cpp
          collect_actions_tests.cpp
          stateidx_tests.cpp
          index_map_tests.cpp
          traits_tests.cpp
          remove_duplicates_tests.cpp
          flatten_transition_table_tests.cpp
          flatten_internal_transition_table_tests.cpp
          dispatch_table_tests.cpp
          switch_tests.cpp
          variant_queue_tests.cpp)

target_link_libraries(hsmUnitTests
        PRIVATE
          hsm::hsm
          gtest_main)

add_test(NAME hsm.UnitTests
        COMMAND hsmUnitTests
)