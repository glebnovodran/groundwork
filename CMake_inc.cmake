set(GCC_RELEASE_COMPILE_FLAGS "-std=c++11 -O3 -mavx2 -mfpmath=sse -ffast-math -ftree-vectorize -mfma")
#set(GCC_RELEASE_LINK_FLAGS    "")

set(GCC_DEBUG_COMPILE_FLAGS "-std=c++11 -ggdb -mavx2 -mfpmath=sse -ffast-math -ftree-vectorize -mfma")
#set(GCC_RELEASE_LINK_FLAGS    "")

set (CMAKE_CXX_FLAGS_RELEASE ${GCC_RELEASE_COMPILE_FLAGS} )
set(CMAKE_EXE_LINKER_FLAGS_RELEASE  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_RELEASE_COMPILE_FLAGS}" )

set (CMAKE_CXX_FLAGS_DEBUG ${GCC_DEBUG_COMPILE_FLAGS})
set(CMAKE_EXE_LINKER_FLAGS_DEBUG  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_DEBUG_COMPILE_FLAGS}" )

set(default_build_type "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" cache
      string "Choose the type of build." FORCE)
  set_property(cache CMAKE_BUILD_TYPE PROPERTY STRINGS
	"Debug" "Release")
	#"Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()