cmake_minimum_required(VERSION 3.5)

include("../../CMake_inc.cmake")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(GW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../groundwork/src")

add_executable(MotEqu
	src/filter.cpp
	src/main.cpp
)

target_include_directories (MotEqu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories (MotEqu PUBLIC ${GW_DIR})

target_link_libraries(MotEqu LINK_PUBLIC Groundwork)