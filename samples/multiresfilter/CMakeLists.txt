cmake_minimum_required(VERSION 3.5)
project("MotEqu" LANGUAGES CXX)

set(GW_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../groundwork")
include("../../CMake_inc.cmake")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(${PROJECT_NAME}
	src/filter.cpp
	src/main.cpp
)

target_include_directories (${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories (${PROJECT_NAME} PUBLIC ${GW_ROOT_DIR}/src)
target_include_directories (${PROJECT_NAME} PUBLIC ${GW_ROOT_DIR}/inc)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC Groundwork)