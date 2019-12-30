/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

class GWScene {
protected:
	GWScene() {};
public:
	struct Config {
		const char* pAppPath;
		const char* pDataDir;
		GWDraw::Interface* pIfc;
	};

	static GWScene* create(const Config& cfg);
	static void destroy(GWScene* pScn);

	void set_draw_interface(GWDraw::Interface* pIfc);
};