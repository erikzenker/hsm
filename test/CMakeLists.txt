cmake_minimum_required(VERSION 3.14)

project(hsm_tests CXX)

# ---- Dependencies ----

set(hsm_INCLUDE_WITHOUT_SYSTEM YES CACHE INTERNAL "")

include(FetchContent)
FetchContent_Declare(hsm SOURCE_DIR "${PROJECT_SOURCE_DIR}/..")
FetchContent_MakeAvailable(hsm)

find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

# ---- Flags ----

include(../cmake/test-flags.cmake)

# ---- Test ----

enable_testing()

add_executable(
    hsmUnitTests 
    unit/main.cpp
    unit/collect_states_tests.cpp
    unit/collect_events_tests.cpp
    unit/collect_actions_tests.cpp
    unit/stateidx_tests.cpp
    unit/index_map_tests.cpp
    unit/traits_tests.cpp
    unit/remove_duplicates_tests.cpp
    unit/flatten_transition_table_tests.cpp
    unit/flatten_internal_transition_table_tests.cpp
    unit/dispatch_table_tests.cpp
    unit/fill_dispatch_table_tests.cpp
    unit/switch_tests.cpp
    unit/variant_queue_tests.cpp
    unit/make_states_map_tests.cpp
    unit/collect_initial_states_tests.cpp
    unit/for_each_tests.cpp
    unit/transition_table_traits_tests.cpp
    unit/chain_actions_tests.cpp
    unit/has_action_tests.cpp
    unit/make_dispatch_tables_tests.cpp
    unit/unexpected_event_handler_tables_tests.cpp
    unit/transition_tests.cpp
)
target_compile_features(hsmUnitTests PRIVATE cxx_std_20)
target_link_libraries(hsmUnitTests PRIVATE hsm::hsm GTest::gtest_main)
gtest_discover_tests(hsmUnitTests TEST_PREFIX unit.)

add_executable(hsmPerformanceTests performance/main.cpp)
target_compile_features(hsmPerformanceTests PRIVATE cxx_std_20)
target_link_libraries(hsmPerformanceTests PRIVATE hsm::hsm GTest::gtest_main)
gtest_discover_tests(hsmPerformanceTests TEST_PREFIX performance.)

add_executable(
    hsmIntegrationTests 
    integration/main.cpp 
    integration/basic_transitions.cpp
    integration/direct_transition.cpp
    integration/entry_exit_actions.cpp
    integration/entry_exit_pseudo_states.cpp
    integration/exit_pseudo_states_with_history.cpp
    integration/anonymous_transition.cpp
    integration/guards_actions.cpp
    integration/orthogonal_regions.cpp
    integration/process_event_return_bool.cpp
    integration/internal_transition.cpp
    integration/unexpected_event_handler.cpp
    integration/history_pseudo_state.cpp
    integration/dependency_injection.cpp
    integration/defer_events.cpp
    integration/transition_postfix_dsl.cpp
    integration/transition_prefix_dsl.cpp
    integration/no_action.cpp
    integration/transition_logging.cpp
    integration/amalgamation_header.cpp
    integration/custom_targets.cpp
    integration/chain_actions.cpp
    integration/reproducer/should_execute_anonymous_transition_once.cpp
    integration/reproducer/should_enter_substate_with_multiple_regions.cpp
)

if (UNIX)
    target_sources(
        hsmIntegrationTests         
        PRIVATE
            # Test compilation fails with msvc with: fatal error C1001: Internal compiler error
            integration/state_data_members.cpp
    )
endif()

target_compile_features(hsmIntegrationTests PRIVATE cxx_std_20)
target_link_libraries(hsmIntegrationTests PRIVATE hsm::hsm GTest::gtest_main)
gtest_discover_tests(hsmIntegrationTests TEST_PREFIX integration.)
