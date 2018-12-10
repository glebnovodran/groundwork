project(Groundwork)
cmake_minimum_required(VERSION 3.5)

include("./CMake_inc.cmake")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory("./samples/multiresfilter")
add_subdirectory("./samples/GLESBasicApp")
add_subdirectory("./tests/sh_pano_test")
add_subdirectory("./tests/calc_test")
add_subdirectory(src)