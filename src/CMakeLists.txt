cmake_minimum_required(VERSION 3.5)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

SET(GCC_RELEASE_COMPILE_FLAGS "-std=c++11 -g -O3 -mavx2 -mfpmath=sse -ffast-math -ftree-vectorize -mfma")
SET(GCC_RELEASE_LINK_FLAGS    "")


SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_RELEASE_COMPILE_FLAGS}" )
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_RELEASE_LINK_FLAGS}" )

SET(TDMOTION_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../TDMotion/src")

add_library(Groundwork STATIC
	GWBase.cpp
	GWVector.cpp
	GWRay.cpp
	GWColor.cpp
	GWMatrix.cpp
	GWTransform.cpp
	GWQuaternion.cpp
	GWMotion.cpp
	GWImage.cpp
	GWSphericalHarmonics.cpp
	GWResource.cpp
	${TDMOTION_DIR}/TDMotion.cpp
)

target_include_directories (Groundwork PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories (Groundwork PUBLIC ${TDMOTION_DIR})

add_executable(gw_test gw_test.cpp)
target_link_libraries(gw_test LINK_PUBLIC Groundwork)