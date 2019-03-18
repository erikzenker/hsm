cmake_minimum_required(VERSION 3.5)
project(hsm VERSION 0.0.1)

# dependencies
find_package(Boost 1.65 REQUIRED)

# library definition
add_library(hsm lib_source.cpp lib_source.h include/hsm/hsm.h)

install(TARGETS hsm EXPORT hsmTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include)
install(EXPORT hsmTargets
        DESTINATION lib/cmake/hsm
        FILE hsmTargets.cmake
        NAMESPACE MyCompany::)
install(DIRECTORY include/hsm
        DESTINATION include)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(hsmConfigVersion.cmake
        COMPATIBILITY SameMajorVersion)
install(FILES hsmConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/hsmConfigVersion.cmake
        DESTINATION lib/cmake/hsm)

target_compile_features(hsm PUBLIC cxx_std_17)
target_include_directories(hsm PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_link_libraries(hsm
        PRIVATE Boost::boost)
add_library(MyCompany::hsm ALIAS hsm)
