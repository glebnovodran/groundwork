cmake_minimum_required(VERSION 3.5)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../../build")
message(STATUS, "CMAKE_BINARY_DIR :")
message(STATUS, ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

SET(GCC_RELEASE_COMPILE_FLAGS "-std=c++11 -g -O3 -mavx -mfpmath=sse -ffast-math -ftree-vectorize -mfma")
SET(GCC_RELEASE_LINK_FLAGS    "")


SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_RELEASE_COMPILE_FLAGS}" )
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_RELEASE_LINK_FLAGS}" )

SET(GW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../groundwork/src")

add_executable(sh_pano_test
	src/main.cpp
)

target_include_directories (sh_pano_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories (sh_pano_test PUBLIC ${GW_DIR})

target_link_libraries(sh_pano_test LINK_PUBLIC Groundwork)