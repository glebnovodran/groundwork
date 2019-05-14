/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#define OGLSYS_ES 1

#if defined(_WIN32)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#if OGLSYS_ES
#	define DYNAMICGLES_NO_NAMESPACE
#	define DYNAMICEGL_NO_NAMESPACE
#	include <DynamicGles.h>
#endif

struct OGLSysIfc {
	void* (*mem_alloc)(size_t size);
	void (*mem_free)(void* p);
	void (*dbg_msg)(const char* pFmt, ...);
	const char* (*load_glsl)(const char* pPath, size_t* pSize);
	void (*unload_glsl)(const char* pCode);

	OGLSysIfc() {
		mem_alloc = nullptr;
		mem_free = nullptr;
		dbg_msg = nullptr;
		load_glsl = nullptr;
		unload_glsl = nullptr;
	}
};

struct OGLSysCfg {
	int x;
	int y;
	int width;
	int height;
	OGLSysIfc ifc;

	OGLSysCfg() : x(0), y(0), width(800), height(600) {}
};

namespace OGLSys {

	void init(const OGLSysCfg& cfg);
	void reset();
	void stop();
	void swap();
	void loop(void(*pLoop)());
	bool valid();

	void bind_def_framebuf();

	int get_width();
	int get_height();
	uint64_t get_frame_count();

	GLuint compile_shader_str(const char* pSrc, size_t srcSize, GLenum kind);
	GLuint compile_shader_file(const char* pSrcPath, GLenum kind);
	GLuint link_prog(GLuint sidVert, GLuint sidFrag);
}
