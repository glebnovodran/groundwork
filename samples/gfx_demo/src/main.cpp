#include <groundwork.hpp>

#include "oglsys.hpp"
#include "demo.hpp"

static const char* s_applicationName = "GW OGL demo";
static SkinAnimDemo s_skinAnimDemo;

static DemoIfc* demos[] = {&s_skinAnimDemo, &s_skinAnimDemo};
static int s_demoNo = 0;

static void init_ogl(int x, int y, int w, int h) {
	OGLSysCfg cfg;
	cfg.x = x;
	cfg.y = y;
	cfg.width = w;
	cfg.height = h;
	cfg.ifc.dbg_msg = GWSys::dbg_msg;
	cfg.withoutCtx = false;
	OGLSys::init(cfg);
}

static void loop() {
	DemoIfc* pDemo = demos[s_demoNo];
	pDemo->loop();
}

int main(int argc, char* argv[]) {
	using namespace std;

	GWApp::init(argc, argv);
	s_demoNo = GWApp::get_int_option("demo", 0);
	size_t numDemos = sizeof(demos)/sizeof(DemoIfc*);
	if (s_demoNo >= numDemos) {
		GWSys::dbg_msg("Bad demo number.");
		return EXIT_FAILURE;
	}

	DemoIfc* pDemo = demos[s_demoNo];

	int w,h;
	pDemo->get_preferred_window_size(w, h);
	init_ogl(0, 0, w, h);
	pDemo->init();
	OGLSys::loop(loop);
	pDemo->reset();
	GWApp::reset();
	return EXIT_SUCCESS;
}