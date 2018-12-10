cmake_minimum_required(VERSION 3.5)

include("../CMake_inc.cmake")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(TDMOTION_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../TDMotion/src")

add_library(Groundwork STATIC
	GWSys.cpp
	GWBase.cpp
	GWList.cpp
	GWVector.cpp
	GWSphere.cpp
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