/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <groundwork.hpp>
#include "oglsys.hpp"
#include "draw.hpp"

static void bind_model(GWModelResource* pMdlRsc) {
}

static void unbind_model(GWModelResource* pMdlRsc) {
}

static void bind_image(GWImage* pImg) {}
static void unbind_image(GWImage* pImg) {}

static struct DrawState {
	enum TransparencyMode {
		OPAQ = 0,
		SEMI = 1
	} mode;
	bool initialized;
	bool doubleSided;

	void set_main_framebuffer() {
		int w = OGLSys::get_width();
		int h = OGLSys::get_height();

		OGLSys::bind_def_framebuf();

		glViewport(0, 0, w, h);
		glScissor(0, 0, w, h);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	};

	void set_opaque() {
		glDisable(GL_BLEND);
		mode = OPAQ;
	};
	void set_semi() {
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		mode = SEMI;
	}

	void set_doublesided() {
		glDisable(GL_CULL_FACE);
		doubleSided = true;
	}
	void set_face_cull() {
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
		glCullFace(GL_BACK);
		doubleSided = false;
	}

	void init() {
		set_main_framebuffer();
		set_opaque();
		set_face_cull();
		initialized = true;
	}
	void reset() {
		if (initialized) {
			OGLSys::bind_def_framebuf();
			initialized = false;
		}
	}
} s_drawState;


static void init() {
	GWResourceUtil::set_model_binding(bind_model, unbind_model);
	GWResourceUtil::set_image_binding(bind_image, unbind_image);

	s_drawState.init();
}

void reset() {
	s_drawState.reset();
}

static int get_screen_height() {
	return OGLSys::get_height();
}

static int get_screen_width() {
	return OGLSys::get_width();
}

static void begin(const GWColorF& clearColor) {
	s_drawState.set_main_framebuffer();

	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

}

static void end() {
	OGLSys::swap();
}

GWDraw::Interface get_ogl_impl() {
	GWDraw::Interface ifc;
	ifc.init = init;
	ifc.reset = reset;
	ifc.begin = begin;
	ifc.end = end;
	ifc.batch = nullptr;
	ifc.get_screen_height = get_screen_height;
	ifc.get_screen_width = get_screen_width;
	return ifc;
}
