# dependencies
if(NOT TARGET hsm::hsm)
    find_package(hsm CONFIG REQUIRED)
endif()

add_executable(complex_euml euml.cpp)
add_executable(complex_sc sc.cpp)
add_executable(complex_switch switch.cpp)
#add_executable(complex_variant variant.cpp)
#set_target_properties(complex_variant 
#        PROPERTIES
#         CXX_STANDARD 17)
add_executable(complex_hsm hsm.cpp)
target_link_libraries(complex_hsm
        PRIVATE
          hsm::hsm)

#add_executable(complex_sml sml.cpp)
