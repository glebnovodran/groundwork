cmake_minimum_required(VERSION 3.5)
project(gfx_demo LANGUAGES CXX)

set(GW_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
include("../../CMake_inc.cmake")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/x11)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/x11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/x11)

add_executable(${PROJECT_NAME}
	${GW_ROOT_DIR}/ext/oglsys/oglsys.cpp
	src/skinanim.cpp
	src/draw_ogl.cpp
	src/main.cpp
)
include("../../CMake_ws.cmake")

target_include_directories (${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories (${PROJECT_NAME} PUBLIC "${GW_ROOT_DIR}/src")
target_include_directories (${PROJECT_NAME} PUBLIC "${GW_ROOT_DIR}/inc")
target_include_directories (${PROJECT_NAME} PUBLIC "${GW_ROOT_DIR}/ext/oglsys")

list(APPEND EXTRA_LIBS Groundwork)
if (EXTRA_LIBS)
	target_link_libraries(${PROJECT_NAME} ${EXTRA_LIBS} )
endif()