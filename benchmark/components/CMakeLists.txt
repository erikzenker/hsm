# dependencies
if(NOT TARGET hsm::hsm)
    find_package(hsm CONFIG REQUIRED)
endif()

function(add_hsm_executable EXECUTABLE_NAME)
  add_executable(${EXECUTABLE_NAME} ${EXECUTABLE_NAME}.cpp)
  set_target_properties(${EXECUTABLE_NAME}
    PROPERTIES
    CXX_STANDARD 17)
  target_link_libraries(${EXECUTABLE_NAME}
    PRIVATE
      hsm::hsm)
endfunction(add_hsm_executable EXECUTABLE_NAME)

add_hsm_executable(fill_dispatch_table_benchmark)
add_hsm_executable(flatten_transition_table_benchmark)
add_hsm_executable(getCombinedStateTypeids_benchmark)
add_hsm_executable(collect_parent_state_typeids_benchmark)
add_hsm_executable(collect_state_typeids_benchmark )
add_hsm_executable(make_index_map_benchmark)
add_hsm_executable(switch_benchmark)