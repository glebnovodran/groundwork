#include <groundwork.hpp>

#include "oglsys.hpp"
#include "demo.hpp"

const char* s_applicationName = "GW OGL demo";
SkinAnimDemo s_skinAnimDemo;

DemoIfc* demos[] = {&s_skinAnimDemo, &s_skinAnimDemo};

struct ProgArgs {
	uint32_t mDemoNo;
	uint32_t mDemoMode;

	ProgArgs() : mDemoNo(0), mDemoMode(0) {}

	void parse(int argc, char* argv[]) {
		mDemoNo = 0;
	}
} s_progArgs;

static void init_ogl(int x, int y, int w, int h) {
	OGLSysCfg cfg;
	cfg.x = x;
	cfg.y = y;
	cfg.width = w;
	cfg.height = h;
	cfg.ifc.dbg_msg = GWSys::dbg_msg;
	OGLSys::init(cfg);
}

static void loop() {
	DemoIfc* pDemo = demos[s_progArgs.mDemoNo];
	pDemo->loop();
}

int main(int argc, char* argv[]) {
	using namespace std;

	s_progArgs.parse(argc, argv);
	size_t numDemos = sizeof(demos)/sizeof(DemoIfc*);
	if (s_progArgs.mDemoNo >= numDemos) { return EXIT_FAILURE; }

	DemoIfc* pDemo = demos[s_progArgs.mDemoNo];

	int w,h;
	pDemo->get_preferred_window_size(w, h);
	init_ogl(0, 0, w, h);
	pDemo->init();
	OGLSys::loop(loop);
	pDemo->reset();
	return EXIT_SUCCESS;
}