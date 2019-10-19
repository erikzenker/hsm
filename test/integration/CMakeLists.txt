cmake_minimum_required(VERSION 3.5)
project(hsmTests)

# dependencies
enable_testing()
find_package(GTest MODULE REQUIRED)
if(NOT TARGET hsm::hsm)
    find_package(hsm CONFIG REQUIRED)
endif()

# target defintion
add_executable(hsmIntegrationTests 
        main.cpp 
        basic_transitions.cpp
        direct_transition.cpp
        entry_exit_actions.cpp
        entry_exit_pseudo_states.cpp
        anonymous_transition.cpp
        guards_actions.cpp
        orthogonal_regions.cpp
)

gtest_add_tests(
        TARGET
          hsmIntegrationTests
        TEST_SUFFIX .noArgs
        TEST_LIST   noArgsTests)

target_link_libraries(hsmIntegrationTests
        PRIVATE
          hsm::hsm
          GTest::Main
)

#add_test(NAME hsm.IntegrationTests
#        COMMAND hsmIntegrationTests
#)
