/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

class GWScene {
protected:
public:
	struct Config {
		const char* pAppPath;
		const char* pDataDir;
		GWRsrcRegistry* pResRgy;
	};

	static GWScene* create(const Config& cfg);
	static void destroy(GWScene* pScn);
};