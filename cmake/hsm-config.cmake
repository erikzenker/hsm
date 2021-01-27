include(CMakeFindDependencyMacro)
find_dependency(Boost 1.72 COMPONENTS boost)

include("${CMAKE_CURRENT_LIST_DIR}/hsm-targets.cmake")
