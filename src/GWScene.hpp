/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

class GWRsrcRegistry;

class GWBundle {
public:
	typedef GWNamedObjList<GWModelResource> MdlRscList;
	typedef GWNamedObjList<GWImage> ImgList;
	typedef GWNamedObjList<GWMotion> MotList;
	typedef GWNamedObjList<GWCollisionResource> ColList;
	typedef GWListItem<GWBundle> Item;
protected:
	Item mItem;
	MdlRscList mMdlLst;
	ImgList mImgLst;
	MotList mMotLst;
	ColList mColLst;
	GWCatalog* mpCat;
	GWRsrcRegistry* mpRegistry;
	std::string mName;
protected:
	friend class GWRsrcRegistry;
	GWBundle() : mpCat(nullptr), mpRegistry(nullptr), mItem(nullptr, this) {}

	void purge_models();
	void purge_images();
	void purge_motions();
	void purge_colli_data();

	static GWBundle* create(const std::string& name, const std::string& dataPath, GWRsrcRegistry* pRgy);
	static void destroy(GWBundle* pBdl);

public:
	const char* get_name() const { return mName.c_str(); }
	void set_name(const std::string& name) {
		mName = name;
		mItem.set_name(mName.c_str());
	}
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
	void unload_bundle(GWBundle* pBdl);
	bool contains_bundle(const GWBundle* pBdl) {
		return (pBdl != nullptr) && (pBdl->mpRegistry == this);
	}
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