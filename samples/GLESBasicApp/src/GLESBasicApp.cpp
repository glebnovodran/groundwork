/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#ifdef _WIN32
#	define WIN32_LEAN_AND_MEAN 1
#	define NOMINMAX
#	define _WIN32_WINNT 0x0500
#	include <tchar.h>
#	include <windows.h>
#elif defined(X11)
	#include "X11/Xlib.h"
	#include "X11/Xutil.h"
#endif

#include <groundwork.hpp>
#include "GLDraw.hpp"

static void data_init(const std::string& gmdlPath) {
}

static void data_reset() {
}

static void view_light_init() {
}

static void main_loop() {
	GLDraw::begin();
	GLDraw::end();
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
#elif defined(UNIX)
int main(int argc, char **argv) {
#endif

	GLDrawCfg cfg;

	cfg.width = 1024;
	cfg.height = 768;
	GLDraw::init(cfg);
	data_init("");
	view_light_init();

	GLDraw::loop(main_loop);

	data_reset();
	GLDraw::reset();

	return 0;
}
