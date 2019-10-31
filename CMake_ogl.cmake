set(POWERVR_SDK_ROOT "/opt/Imagination/PowerVR_Graphics/PowerVR_SDK/SDK_2019_R1.1")
set(GLSL_VALIDATOR "/projects/VulkanSDK/1.1.114.0/x86_64/bin/glslangValidator")

set(GPU_DST_DIR ${CMAKE_BINARY_DIR}/bin/data/gpu)

target_include_directories (${PROJECT_NAME} PUBLIC "${POWERVR_SDK_ROOT}/include")
target_include_directories (${PROJECT_NAME} PUBLIC "${POWERVR_SDK_ROOT}/external/glm")

add_custom_command(OUTPUT ${GPU_DST_DIR}/gpu.done
	COMMAND export GLSL=${GLSL_VALIDATOR} && export GPU_DST_DIR=${GPU_DST_DIR}
	&& ${CMAKE_CURRENT_SOURCE_DIR}/src/gpu/ogl/mk.sh
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/gpu/ogl"
)

add_custom_target(${PROJECT_NAME}_gpu
	DEPENDS ${GPU_DST_DIR}/gpu.done
)

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_gpu)