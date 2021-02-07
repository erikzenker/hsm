cmake_minimum_required(VERSION 3.14)

project(hsm_benchmarks CXX)

# ---- Dependencies ----

set(hsm_INCLUDE_WITHOUT_SYSTEM YES CACHE INTERNAL "")

include(FetchContent)
FetchContent_Declare(hsm SOURCE_DIR "${PROJECT_SOURCE_DIR}/..")
FetchContent_MakeAvailable(hsm)

# ---- Flags ----

include(../cmake/test-flags.cmake)

# ---- Benchmarks ----

set(
    components_benchmarks
    fill_dispatch_table_benchmark
    flatten_transition_table_benchmark
    getCombinedStateTypeids_benchmark
    collect_parent_state_typeids_benchmark
    collect_state_typeids_benchmark 
    make_index_map_benchmark
    collect_events_recursive_benchmark
    remove_duplicate_types_benchmark
    switch_benchmark
    index_of_benchmark
)
set(simple_benchmarks hsm sc euml)
set(complex_benchmarks hsm sc euml switch)

foreach(list IN ITEMS components simple complex)
  foreach(name IN LISTS "${list}_benchmarks")
    set(target "${list}_${name}")
    add_executable("${target}" "${list}/${name}.cpp")
    target_compile_features("${target}" PRIVATE cxx_std_17)
    target_link_libraries("${target}" PRIVATE hsm::hsm)
  endforeach()
endforeach()
