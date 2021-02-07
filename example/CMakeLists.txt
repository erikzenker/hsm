cmake_minimum_required(VERSION 3.14)

project(hsm_examples CXX)

# ---- Dependencies ----

set(hsm_INCLUDE_WITHOUT_SYSTEM YES CACHE INTERNAL "")

include(FetchContent)
FetchContent_Declare(hsm SOURCE_DIR "${PROJECT_SOURCE_DIR}/..")
FetchContent_MakeAvailable(hsm)

# ---- Flags ----

include(../cmake/test-flags.cmake)

# ---- Examples ----

foreach(name IN ITEMS cdplayer turnstile)
  add_executable("${name}" "${name}/main.cpp")
  target_compile_features("${name}" PRIVATE cxx_std_17)
  target_link_libraries("${name}" PRIVATE hsm::hsm)
endforeach()
