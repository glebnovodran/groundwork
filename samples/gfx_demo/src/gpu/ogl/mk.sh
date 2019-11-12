#!/bin/bash

function shader() {
	local outname=$1
	echo $outname
	shift
	while test ${#} -gt 0
	do
		echo "	${1}"
		cat ${1} >> $outname
		shift
	done
	${GLSL} $outname
}

function frag_shader() {
	echo "frag shader"
	echo $@:0
	shader $1 prologue_frag.h "${@:2}"
}

function vtx_shader() {
	echo "vtx shader"
	shader $1 prologue_vert.h "${@:2}"
}

echo "Shader validation at ..."
pwd
#echo "${GLSL}"
if [ -z "$GPU_DST_DIR" ]; then
	echo "GPU_DST_DIR is not set"
	export GPU_DST_DIR=./out
	echo "Exporting to ${GPU_DST_DIR}"
	mkdir ${GPU_DST_DIR}
fi

rm ${GPU_DST_DIR}/*

if [ -z "${GLSL}" ]; then
	echo "Path to GLSL validator is not set"
	export GLSL=/projects/VulkanSDK/1.1.114.0/x86_64/bin/glslangValidator
fi
#else
	echo "Using $GLSL"
	vtx_shader ${GPU_DST_DIR}/skin.vert skin.vert
	frag_shader ${GPU_DST_DIR}/mtl.frag simple.frag
	touch ${GPU_DST_DIR}/gpu.done
#fi

