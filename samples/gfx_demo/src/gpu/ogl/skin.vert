#define MAX_XFORMS 128

layout(std140) uniform gpCam {
	mat4 gpView;
	mat4 gpProj;
	mat4 gpViewProj;

	mat4 gpInvView;
	mat4 gpInvProj;
	mat4 gpViewProj;
	vec3 gpViewPos;
};

layout(std140) uniform gpObj{
	vec4 gpWorldXform[3];
};

layout(std140) uniform gpSkin {
	vec4 gpSkinXform[MAX_XFORMS*3];
};

in vec4 vtxPosAO;
in vec4 vtxOctNrmTng;
in vec4 vtxClr;
in vec4 vtxTex; // uv, uv2
in vec4 vtxWgt;
in ivec4 vtxJIdx;

out vec3 outPos;
out vec3 outNrm;
out vec3 outTng;
out vec3 outBiNrm;
out vec2 outTex;
out vec2 outTex2;
out vec4 outClr;
out float outAO;

void get_xform(out vec4 wm[3]) {
	//...
}

void main() {
	
}