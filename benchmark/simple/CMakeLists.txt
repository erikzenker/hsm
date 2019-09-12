# dependencies
if(NOT TARGET hsm::hsm)
    find_package(hsm CONFIG REQUIRED)
endif()



add_executable(simple_euml euml.cpp)
add_executable(simple_sc sc.cpp)
add_executable(simple_hsm hsm.cpp)
target_link_libraries(simple_hsm
        PRIVATE
          hsm::hsm)



#add_executable(simple_sml sml.cpp)
