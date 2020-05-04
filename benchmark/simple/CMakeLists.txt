# dependencies
if(NOT TARGET hsm::hsm)
    find_package(hsm CONFIG REQUIRED)
endif()



add_executable(simple_euml euml.cpp)
set_target_properties(simple_euml
          PROPERTIES
           CXX_STANDARD 17)
target_link_libraries(simple_euml Boost::boost)

add_executable(simple_sc sc.cpp)
set_target_properties(simple_sc
          PROPERTIES
           CXX_STANDARD 17)
target_link_libraries(simple_sc Boost::boost)
add_executable(simple_hsm hsm.cpp)
set_target_properties(simple_hsm
          PROPERTIES
           CXX_STANDARD 17)
target_link_libraries(simple_hsm
        PRIVATE
          hsm::hsm)



add_executable(simple_sml sml.cpp)
