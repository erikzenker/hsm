cmake_minimum_required(VERSION 3.5)
project(hsm VERSION 0.0.1)

# library definition
add_library(hsm INTERFACE)

install(TARGETS hsm EXPORT hsmTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include)
install(EXPORT hsmTargets
        DESTINATION lib/cmake/hsm
        FILE hsmTargets.cmake
        NAMESPACE hsm::)
install(DIRECTORY include/hsm
        DESTINATION include)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(hsmConfigVersion.cmake
        COMPATIBILITY SameMajorVersion)
install(FILES hsmConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/hsmConfigVersion.cmake
        DESTINATION lib/cmake/hsm)

#set_target_properties(hsm PROPERTIES LINKER_LANGUAGE CXX)
target_compile_features(hsm INTERFACE cxx_std_17)
target_include_directories(hsm INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_link_libraries(hsm
        INTERFACE Boost::boost)
add_library(hsm::hsm ALIAS hsm)
