/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

class GWBundle {
public:
	typedef GWNamedObjList<GWModelResource> MdlRscList;
	typedef GWNamedObjList<GWImage> ImgList;
	typedef GWNamedObjList<GWMotion> MotList;
	typedef GWNamedObjList<GWCollisionResource> ColList;

protected:
	MdlRscList mMdlLst;
	ImgList mImgLst;
	MotList mMotLst;
	ColList mColLst;
	GWCatalog* mpCat;
	std::string mName;
protected:
	GWBundle() : mpCat(nullptr) {}

	static GWBundle* create(const std::string& name, const std::string& dataPath, GWCatalog* pCat);

	void purge_models();
	void purge_images();
	void purge_motions();
	void purge_colli_data();

public:
	const char* get_name() const { return mName.c_str(); }

	GWModelResource* find_model(const std::string& name) {
		return mMdlLst.find_first_val(name.c_str());
	}
	GWImage* find_image(const std::string& name) {
		return mImgLst.find_first_val(name.c_str());
	}
	GWMotion* find_motion(const std::string& name) {
		return mMotLst.find_first_val(name.c_str());
	}
	GWCollisionResource* find_colli_data(const std::string& name) {
		return mColLst.find_first_val(name.c_str());
	}

	static GWBundle* create(const std::string& name, const std::string& dataPath);
	static void destroy(GWBundle* pBdl);
};

class GWRsrcRegistry {
public:
	typedef GWNamedObjList<GWBundle> BundleList;

protected:
	BundleList mBdlLst;
	std::string mDataPath;
protected:
	GWRsrcRegistry() {}
public:
	GWBundle* find_bundle(const std::string& name) {
		return mBdlLst.find_first_val(name.c_str());
	}
	GWBundle* load_bundle(const std::string& name);
	void unload_bundle(const std::string& name);

	inline GWModelResource* find_model(const std::string& bdlName, const std::string& mdlName) {
		GWBundle* pBdl = find_bundle(bdlName);
		return pBdl == nullptr ? nullptr : pBdl->find_model(mdlName);
	}

	inline GWModelResource* find_model(GWBundle* pBdl, const std::string& mdlName) {
		return pBdl == nullptr ? nullptr : pBdl->find_model(mdlName);
	}

	static GWRsrcRegistry* create(const std::string& appPath, const std::string& relDataDir);
	static void destroy(GWRsrcRegistry* pRgy);

};

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