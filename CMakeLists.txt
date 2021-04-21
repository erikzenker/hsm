cmake_minimum_required(VERSION 3.14)

project(
    hsm
    VERSION 0.0.1
    DESCRIPTION "Finite state machine library based on Boost.Hana"
    HOMEPAGE_URL "https://github.com/erikzenker/hsm"
    LANGUAGES CXX
)

# ---- Warning guard ----

# Protect dependents from this project's warnings if the guard isn't disabled
set(hsm_warning_guard SYSTEM)
if(hsm_INCLUDE_WITHOUT_SYSTEM)
  set(hsm_warning_guard "")
endif()

# ---- Declare library ----

add_library(hsm INTERFACE)
add_library(hsm::hsm ALIAS hsm)

target_include_directories(
    hsm
    ${hsm_warning_guard}
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(hsm INTERFACE cxx_std_17)

find_package(Boost 1.72 REQUIRED)
target_link_libraries(hsm INTERFACE Boost::boost)

# ---- Install ----

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(hsm_include_directory "${CMAKE_INSTALL_INCLUDEDIR}")

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
    DESTINATION "${hsm_include_directory}"
    COMPONENT hsm_Development
)

install(
    TARGETS hsm
    EXPORT hsm-targets
    INCLUDES DESTINATION "${hsm_include_directory}"
)

write_basic_package_version_file(
    hsm-config-version.cmake
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

set(hsm_install_cmakedir "${CMAKE_INSTALL_LIBDIR}/cmake/hsm")

install(
    FILES
    "${PROJECT_SOURCE_DIR}/cmake/hsm-config.cmake"
    "${PROJECT_BINARY_DIR}/hsm-config-version.cmake"
    DESTINATION "${hsm_install_cmakedir}"
    COMPONENT hsm_Development
)

install(
    EXPORT hsm-targets
    NAMESPACE hsm::
    DESTINATION "${hsm_install_cmakedir}"
    COMPONENT hsm_Development
)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  include(CPack)
endif()
