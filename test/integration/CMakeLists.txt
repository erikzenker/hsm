cmake_minimum_required(VERSION 3.5)
project(hsmTests)

# dependencies
enable_testing()
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
        internal_transition.cpp
        unexpected_transition_handler.cpp
        history_pseudo_state.cpp
        dependency_injection.cpp
        defer_events.cpp
        transition_dsl.cpp
)



target_link_libraries(hsmIntegrationTests
        PRIVATE
          hsm::hsm
          gtest_main
)

#add_test(NAME hsm.IntegrationTests
#        COMMAND hsmIntegrationTests
#)
