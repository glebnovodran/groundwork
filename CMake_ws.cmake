# Window system
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