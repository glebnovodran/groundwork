cmake_minimum_required(VERSION 3.5)
project(TDGeoViewer LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/x11)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/x11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/x11)
set(POWERVR_SDK_ROOT "/opt/Imagination/PowerVR_Graphics/PowerVR_SDK/SDK_2018_R1")
#set(GCC_RELEASE_COMPILE_FLAGS "-std=c++11 -g -O3 -mavx -mfpmath=sse -ffast-math -ftree-vectorize -mfma -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -lglib-2.0 -DNDEBUG")
set(GCC_RELEASE_COMPILE_FLAGS "-std=c++11 -g -O3 -mavx -mfpmath=sse -ffast-math -ftree-vectorize -mfma")
set(GCC_RELEASE_LINK_FLAGS    "")

set(GCC_DEBUG_COMPILE_FLAGS "-std=c++11 -g -ggdb -mavx -mfpmath=sse -ffast-math -ftree-vectorize -mfma")


if (UNIX)
	add_definitions(-DUNIX)
	set(EXTRA_LIBS ${CMAKE_DL_LIBS})

	if (NOT WS) #We support building for several Windowing Systems. Typical desktop systems support X11 and XCB, Wayland is catching on. NullWS is used by some development platforms/ testchip.
		message("WS Variable not set. If you wish to build for X11, Wayland or another supported windowing system, please pass -DWS=X11 or -DWS=Wayland etc. to CMake")
	endif()
	if (${WS} STREQUAL X11)
		find_package(X11 REQUIRED)
		list(APPEND EXTRA_LIBS ${X11_LIBRARIES})
		include_directories(${X11_INCLUDE_DIR})
		add_definitions(-DX11)
	endif()

endif()

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_RELEASE_COMPILE_FLAGS}" )
#set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_DEBUG_COMPILE_FLAGS}" )
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_RELEASE_LINK_FLAGS}" )

set(GW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../groundwork/src")

add_executable(GLESBasicApp
	src/GLDraw.cpp
	src/GLESBasicApp.cpp
)

target_include_directories (GLESBasicApp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories (GLESBasicApp PUBLIC ${GW_DIR})

target_include_directories (GLESBasicApp PUBLIC "${POWERVR_SDK_ROOT}/include")
target_include_directories (GLESBasicApp PUBLIC "${POWERVR_SDK_ROOT}/external/glm")

#target_link_libraries(GLESBasicApp LINK_PUBLIC Groundwork)
list(APPEND EXTRA_LIBS Groundwork)
if (EXTRA_LIBS)
	target_link_libraries(GLESBasicApp ${EXTRA_LIBS} )
endif()