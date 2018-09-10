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

#include <sstream>

#include <groundwork.hpp>
#include "GLDraw.hpp"

std::string folder_from_file_path(const std::string& path, char sep = '/') {
	using namespace std;
	int idx = path.rfind(sep, path.length());
	if (string::npos != idx) {
		return path.substr(0, idx+1);
	}
	return "";
}
static GWModelResource* s_pMdlRsrc = nullptr;

static bool data_init(const std::string& catPath) {
	using namespace std;
	bool isLoaded = true;
	GWCatalog* pCat = GWCatalog::load(catPath);
	if (pCat) {
		string folder = folder_from_file_path(catPath);
		uint32_t n = pCat->mNum;
		for (uint32_t i = 0; i < n; ++i) {
			const char* pFileName = pCat->get_file_name(i);
			const char* pName = pCat->get_name(i);
			GWResourceKind kind = pCat->get_file_kind(i);
			GWSys::dbg_msg("%s : %s\n", pName, GWCatalog::get_kind_string(kind));

			switch (kind) {
			case GWResourceKind::MODEL: {
					GWModelResource* pMdlRsc = GWModelResource::load(folder + pFileName);
					if (pMdlRsc == nullptr) {
						GWSys::dbg_msg("Couldn't load resource");
						return false;
					} else {
						GLDraw::Model* pMdl = GLDraw::create_model(*pMdlRsc, pName);
					}
				}
				break;
			case GWResourceKind::DDS:
				GLDraw::create_texture((folder + pFileName).c_str(), pName);
				break;
			case GWResourceKind::TDMOT:
				break;
			default:
				break;
			}
		}
	}

	return isLoaded;
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
	using namespace std;
	string args = pCmdLine;

#elif defined(UNIX)
int main(int argc, char **argv) {
	using namespace std;
	std::string args = argc < 2 ? "" : argv[1];
#endif

	if (args.empty()) {
		return -1;
	}

	istringstream ss(args);
	string path;
	getline(ss, path, ' ');

	GLDrawCfg cfg;

	cfg.width = 1024;
	cfg.height = 768;
	GLDraw::init(cfg);
	if (data_init(path)) {
		view_light_init();
		GLDraw::loop(main_loop);
	}
	data_reset();
	GLDraw::reset();

	return 0;
}
