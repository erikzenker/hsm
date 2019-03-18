cmake_minimum_required(VERSION 3.5)
project(hsm)

# add project code
add_subdirectory(src)

# add unit tests
enable_testing()
add_subdirectory(test)