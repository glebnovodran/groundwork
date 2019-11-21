cmake_minimum_required(VERSION 3.5)
project(calc_test LANGUAGES CXX)

set(GW_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
include("../../CMake_inc.cmake")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(${PROJECT_NAME}
	src/test_quat.cpp
	src/test_mtx.cpp
	src/test_xform.cpp
	src/test.cpp
	src/main.cpp
)

target_include_directories (${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories (${PROJECT_NAME} PUBLIC "${GW_ROOT_DIR}/src")
target_include_directories (${PROJECT_NAME} PUBLIC "${GW_ROOT_DIR}/inc")

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC Groundwork)