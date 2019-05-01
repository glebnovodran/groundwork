cmake_minimum_required(VERSION 3.5)
project(gl_demo LANGUAGES CXX)

include("../../CMake_inc.cmake")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/x11)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/x11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/x11)
set(POWERVR_SDK_ROOT "/opt/Imagination/PowerVR_Graphics/PowerVR_SDK/SDK_2018_R2")

if (UNIX)
	add_definitions(-DUNIX)
	set(EXTRA_LIBS ${CMAKE_DL_LIBS})

	if (NOT WS)
		message("WS Variable not set. Please pass -DWS=X11 or -DWS=Wayland (not supported yet) etc. to CMake")
	endif()
	if (${WS} STREQUAL X11)
		find_package(X11 REQUIRED)
		list(APPEND EXTRA_LIBS ${X11_LIBRARIES})
		include_directories(${X11_INCLUDE_DIR})
		add_definitions(-DX11)
	endif()

endif()

set(GW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../groundwork/src")

add_executable(gl_demo
	src/main.cpp
	src/demo_skin_anim.cpp
	src/oglsys.cpp
)

target_include_directories (gl_demo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories (gl_demo PUBLIC ${GW_DIR})

target_include_directories (gl_demo PUBLIC "${POWERVR_SDK_ROOT}/include")
target_include_directories (gl_demo PUBLIC "${POWERVR_SDK_ROOT}/external/glm")

list(APPEND EXTRA_LIBS Groundwork)
if (EXTRA_LIBS)
	target_link_libraries(gl_demo ${EXTRA_LIBS} )
endif()