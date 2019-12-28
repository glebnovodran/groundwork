/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#define GW_RSRC_SIG "rsrc:"
#define GW_RSRC_ID(_name) GW_RSRC_SIG _name

class GWRsrcRegistry;
class GWBundle;

enum class GWResourceKind {
	UNKNOWN = -1,
	// native
	CATALOG = 0,
	MODEL = 1,
	COL_DATA = 2,
	// foreign
	DDS = 0x100,
	TDMOT = 0x101,
	TDGEO = 0x102
};

class GWResource {
public:
	/* +00 */ char mSignature[0x10];
	/* +10 */ uint32_t mVersion;
	/* +14 */ uint32_t mDataSize;
	/* +18 */ uint32_t mStrsTop;
	/* +1C */ uint32_t mStrsSize;

	struct Binding {
		void* pMem;
	};

	Binding* get_binding_ptr() {
		return reinterpret_cast<Binding*>(GWBase::incr_ptr(this, mDataSize));
	}

	Binding get_binding() {
		Binding* pBinding = get_binding_ptr();
		Binding bnd;
		// since Binding was allocated unaligned
		std::memcpy(&bnd, pBinding, sizeof(Binding));
		return bnd;
	}
	void set_binding(const Binding& bnd) {
		Binding* pBinding = get_binding_ptr();
		std::memcpy(pBinding, &bnd, sizeof(Binding));
	}

	const char* get_str(uint32_t offs = 0) const {
		if (offs < mStrsSize) {
			return reinterpret_cast<const char*>(this) + mStrsTop + offs;
		}
		return nullptr;
	}

	void* get_ptr(uint32_t offs) {
		if (offs < mDataSize) {
			return reinterpret_cast<char*>(this) + offs;
		}
		return nullptr;
	}

	static GWResource* load(const std::string& path, const char* pSig);
	static void unload(GWResource* pRsrc);

	bool binding_memory_allocated() {
		Binding bnd = get_binding();
		return bnd.pMem != nullptr;
	}
	void alloc_binding_memory(uint32_t size) {
		Binding bnd = get_binding();
		bnd.pMem = new char[size];
		set_binding(bnd);
	}
	void release_binding_memory() {
		Binding bnd = get_binding();
		if (bnd.pMem != nullptr) {
			delete[] (char*)bnd.pMem;
			bnd.pMem = nullptr;
		}
	}
	void set_binding_memory(void* pMem) {
		Binding bnd = get_binding();
		bnd.pMem = pMem;
		set_binding(bnd);
	}
	template<typename T> T* get_binding_memory() {
		Binding bnd = get_binding();
		return reinterpret_cast<T*>(bnd.pMem);
	}
};

class GWModelResource : public GWResource {
public:
	/* +20 */ uint32_t mPathOffs;
	/* +24 */ uint32_t mNumPnt;
	/* +28 */ uint32_t mNumTri;
	/* +2C */ uint32_t mNumMtl;
	/* +30 */ uint32_t mNumIdx16;
	/* +34 */ uint32_t mNumIdx32;
	/* +38 */ uint32_t mNumSkinNodes;
	/* +3C */ uint32_t mNumSkelNodes;
	/* +40 */ uint32_t mOffsPnts;
	/* +44 */ uint32_t mOffsAttrs;
	/* +48 */ uint32_t mOffsMtls;
	/* +4C */ uint32_t mOffsIdx16;
	/* +50 */ uint32_t mOffsIdx32;
	/* +54 */ uint32_t mOffsSkin;
	/* +58 */ uint32_t mOffsSkel;
	/* +5C */ uint32_t mAttrMask;
	/* +60 */ uint32_t mOffsExtInfo;

	struct Attr {
		GWHalf4 mNrmTngEnc;
		GWHalf4 mColor;
		GWHalf4 mTex;

		GWVectorF get_normal() const {
			GWVectorF vec;
			float oct[2];
			GWBase::half_to_float(oct, (uint16_t*)&mNrmTngEnc, 2);
			GWBase::oct_to_vec(oct[0], oct[1], vec.x, vec.y, vec.z);
			return vec;
		}

		GWVectorF get_tangent() const {
			GWVectorF vec;
			float oct[2];
			GWBase::half_to_float(oct, (uint16_t*)&mNrmTngEnc.elems[2], 2);
			GWBase::oct_to_vec(oct[0], oct[1], vec.x, vec.y, vec.z);
			return vec;
		}

		GWColorF get_color() const {
			GWColorF clr;
			clr.from_tuple(mColor.get());
			return clr;
		}

		GWColorTuple3f get_rgb() const {
			GWColorTuple3f rgb;
			GWBase::half_to_float(&rgb.elems[0], (uint16_t*)&mColor, 3);
			return rgb;
		}

		GWTuple4f get_tex() const {
			return mTex.get();
		}

		GWTuple2f get_uv() const {
			GWTuple2f uv;
			GWBase::half_to_float(&uv.elems[0], (uint16_t*)&mTex, 2);
			return uv;
		}

		GWTuple2f get_uv2() const {
			GWTuple2f uv2;
			GWBase::half_to_float(&uv2.elems[0], (uint16_t*)&mTex.elems[2], 2);
			return uv2;
		}
	};

	struct MtlFlags {
		uint32_t val;

		bool get(int idx) const { return !!(val & (1 << idx)); }
		bool is_double_sided() const { return get(0); }
		bool is_semi_transparent() const { return get(1); }
		bool get_tangent_flip_flag() const { return get(2); }
		bool get_bitangent_flip_flag() const { return get(3); }
	};

	struct IdxInfo {
		int32_t mOrg;
		uint32_t mNumTri;
		int32_t mMin;
		int32_t mMax;

		bool is_idx16() const { return (mMax - mMin) < (1 << 16); }
	};

	struct Material {
		uint32_t mPathOffs;
		MtlFlags mFlags;
		uint32_t mBaseMapPathOffs;
		uint32_t mExtParamsOffs;
		IdxInfo mIdx;
		GWColorTuple3f mBaseColor;
		GWColorTuple3f mSpecColor;
		float mRoughness;
		float mFresnel;
		float mBumpScale;
	};

	static const uint32_t NONE = (uint32_t)-1;

	const char* get_path() const { return get_str(mPathOffs); }

	bool check_attr_flag(int flagIdx) const { return !!(mAttrMask & (1 << flagIdx)); }

	bool valid_nrm() const { return check_attr_flag(0); }
	bool valid_tng() const { return check_attr_flag(1); }
	bool valid_rgb() const { return check_attr_flag(2); }
	bool valid_uv() const { return check_attr_flag(3); }
	bool valid_uv2() const { return check_attr_flag(4); }
	bool valid_alpha() const { return check_attr_flag(5); }
	bool valid_ao() const { return check_attr_flag(6); }
	bool alpha_is_ao() const { return !valid_alpha() && valid_ao(); }

	bool has_skin() const { return mNumSkinNodes > 0; }
	bool has_skel() const { return mNumSkelNodes > 0; }

	bool check_skin_node_idx(uint32_t idx) const { return idx < mNumSkinNodes; }
	bool check_skel_node_idx(uint32_t idx) const { return idx < mNumSkelNodes; }

	bool check_pnt_idx(uint32_t idx) const { return idx < mNumPnt; }

	float* get_pnt_ptr(uint32_t idx) {
		float* p = nullptr;
		if (check_pnt_idx(idx)) {
			p = reinterpret_cast<float*>(get_ptr(mOffsPnts)) + (idx * 3);
		}
		return p;
	}

	uint16_t* get_idx16() {
		return mNumIdx16 > 0 ? reinterpret_cast<uint16_t*>(get_ptr(mOffsIdx16)) : nullptr;
	}
	uint32_t* get_idx32() {
		return mNumIdx32 > 0 ? reinterpret_cast<uint32_t*>(get_ptr(mOffsIdx32)) : nullptr;
	}

	GWVectorF get_pnt(uint32_t idx) {
		GWVectorF pnt(0);
		float* p = get_pnt_ptr(idx);
		if (p != nullptr) {
			pnt.x = p[0];
			pnt.y = p[1];
			pnt.z = p[2];
		}
		return pnt;
	}

	Attr* get_attr(uint32_t idx) {
		Attr* pAttr = nullptr;
		if (check_pnt_idx(idx)) {
			pAttr = reinterpret_cast<Attr*>(get_ptr(mOffsAttrs)) + idx;
		}
		return pAttr;
	}

	float get_pnt_alpha(uint32_t idx) {
		float alpha = 1.0f;
		if (valid_alpha()) {
			Attr* pAttr = get_attr(idx);
			if (pAttr) {
				alpha = pAttr->mColor[3].get();
			}
		}
		return alpha;
	}

	float get_pnt_ao(uint32_t idx) {
		float occl = 1.0f;
		if (valid_ao()) {
			Attr* pAttr = get_attr(idx);
			if (pAttr) {
				occl = pAttr->mColor[3].get();
			}
		}
		return occl;
	}

	Material* get_mtl(uint32_t idx) {
		Material* pMtl = nullptr;
		if (idx < mNumMtl) {
			pMtl = reinterpret_cast<Material*>(get_ptr(mOffsMtls)) + idx;
		}
		return pMtl;
	}

	const char* get_mtl_path(uint32_t idx) {
		const char* pPath = nullptr;
		Material* pMtl = get_mtl(idx);
		if (pMtl) {
			pPath = get_str(pMtl->mPathOffs);
		}
		return pPath;
	}

	const char* get_mtl_name(uint32_t idx);

	const char* get_mtl_basemap_name(uint32_t idx) {
		Material* pMtl = get_mtl(idx);
		if (pMtl != nullptr) {
			if (pMtl->mPathOffs != NONE) {
				return get_str(pMtl->mBaseMapPathOffs);
			}
		}
		return nullptr;
	}

	char* get_mtl_ext_params(uint32_t idx) {
		Material* pMtl = get_mtl(idx);
		uint32_t offs = pMtl->mExtParamsOffs;
		return reinterpret_cast<char*>(get_ptr(offs));
	}

	GWTransformF get_skel_node_local_mtx(uint32_t idx) {
		GWTransformF lm;
		lm.set_identity();
		if (check_skel_node_idx(idx)) {
			lm = reinterpret_cast<GWTransformF*>(get_ptr(mOffsSkel))[idx];
		}
		return lm;
	}

	GWTransformF calc_skel_node_world_xform(uint32_t idx, const GWTransformF* pLM = nullptr, GWTransformF* pParentWM = nullptr);

	uint32_t* get_skel_node_name_offs() {
		return reinterpret_cast<uint32_t*>(
			&reinterpret_cast<GWTransformF*>(get_ptr(mOffsSkel))[mNumSkelNodes]
			);
	}

	uint32_t* get_skel_node_parent_indices() {
		return &get_skel_node_name_offs()[mNumSkelNodes];
	}

	const char* get_skel_node_name(uint32_t idx) {
		const char* pName = nullptr;
		if (check_skel_node_idx(idx)) {
			pName = get_str(get_skel_node_name_offs()[idx]);
		}
		return pName;
	}

	uint32_t get_skel_node_parent_idx(uint32_t idx) {
		uint32_t parentIdx = NONE;
		if (check_skel_node_idx(idx)) {
			parentIdx = get_skel_node_parent_indices()[idx];
		}
		return parentIdx;
	}

	uint32_t* get_skin_to_skel_map() { return &reinterpret_cast<uint32_t*>(get_ptr(mOffsSkin))[mNumSkinNodes]; }

	uint32_t find_skel_node_skin_idx(uint32_t skelIdx);

	bool is_skel_node_skin_deformer(uint32_t skelIdx) { return check_skin_node_idx(find_skel_node_skin_idx(skelIdx)); }

	const char* get_skin_node_name(uint32_t skinIdx);

	void* get_skin_data() {
		return &reinterpret_cast<uint32_t*>(get_ptr(mOffsSkin))[mNumSkinNodes * 2];
	}

	GWTuple4u get_pnt_skin_joints(uint32_t pntIdx);
	GWTuple4f get_pnt_skin_weights(uint32_t pntIdx);
	uint32_t get_pnt_skin_joints_count(uint32_t pntIdx);

	GWSphereF calc_skin_node_sphere_of_influence(uint32_t skinIdx, GWVectorF* pMem = nullptr);
	GWSphereF* calc_skin_spheres_of_influence();

	static GWModelResource* load(const std::string& path);

	void write_geo(std::ostream& os);
	void save_geo(const std::string& path) {
		std::ofstream os(path);
		if (os.bad()) return;
		write_geo(os);
		os.close();
	}

	void write_skel(std::ostream& os, const char* pBase = nullptr);
	void save_skel(const std::string& path) {
		if (!has_skel()) return;
		std::ofstream os(path);
		if (os.bad()) return;
		write_skel(os);
		os.close();
	}
};

class GWCollisionResource : public GWResource {
public:
	/* +20 */ uint32_t mPathOffs;
	/* +24 */ int32_t mNumPnt;
	/* +28 */ int32_t mNumPol;
	/* +2C */ int32_t mOffsPnts;
	/* +30 */ int32_t mOffsPols;
	/* +34 */ int32_t mOffsTris;
	/* +38 */ int32_t mOffsIdx;
	/* +3C */ int32_t mOffsBVH;
	/* +40 */ GWVectorF mBBoxMin;
	/* +4C */ GWVectorF mBBoxMax;

	struct Poly {
		GWVectorF mBBoxMin;
		GWVectorF mBBoxMax;
		GWVectorF mNormal;
		int32_t mOffsIdx;
		int32_t mNumVtx;
		int32_t mOffsTris;
	};

	struct BVHNode {
		GWVectorF mBBoxMin;
		GWVectorF mBBoxMax;
		int32_t mLeft;
		int32_t mRight;

		bool is_leaf() const { return mRight < 0; }
		int get_poly_id() const { return is_leaf() ? mLeft : -1; }
	};

	class TriFunc {
	public:
		TriFunc() {}
		virtual ~TriFunc() {}
		virtual void operator ()(GWCollisionResource& cls, GWVectorF vtx[3], GWVectorF nrm, int polIdx, int triIdx) {}
	};

	const char* get_path() const { return get_str(mPathOffs); }

	GWVectorF* get_pnts_top() {
		return reinterpret_cast<GWVectorF*>(get_ptr(mOffsPnts));
	}

	Poly* get_pols_top() {
		return reinterpret_cast<Poly*>(get_ptr(mOffsPols));
	}

	int32_t* get_idx_top() {
		return reinterpret_cast<int32_t*>(get_ptr(mOffsIdx));
	}

	int32_t* get_tris_top() {
		return reinterpret_cast<int32_t*>(get_ptr(mOffsTris));
	}

	BVHNode* get_bvh_top() {
		return reinterpret_cast<BVHNode*>(get_ptr(mOffsBVH));
	}

	int calc_num_tris();

	bool check_poly_idx(int polIdx) const { return polIdx >= 0 && polIdx < mNumPol; }
	bool get_poly_tri(GWVectorF vtx[3], int polIdx, int triIdx);
	int get_poly_num_tris(int polIdx);
	int for_all_tris(TriFunc& func, bool withNormals = true);

	void write_geo(std::ostream& os);
	void save_geo(const std::string& path);

	void write_tri_geo(std::ostream& os);
	void save_tri_geo(const std::string& path);

	void write_bvh_geo(std::ostream& os);
	void save_bvh_geo(const std::string& path);

	static GWCollisionResource* load(const std::string& path);
};

class GWCatalog : public GWResource {
public:
	/* +20 */ uint32_t mNum;
	/* +24 */ struct Entry {
				uint32_t mKind;
				int32_t mNameOffs;
				int32_t mFileNameOffs;
			} mList[1];

	bool check_idx(uint32_t idx) const { return idx < mNum; }
	const char* get_name(uint32_t idx) const { return check_idx(idx) ? get_str(mList[idx].mNameOffs) : nullptr; }
	const char* get_file_name(uint32_t idx) const { return check_idx(idx) ? get_str(mList[idx].mFileNameOffs) : nullptr; }
	GWResourceKind get_kind(uint32_t idx) const { return check_idx(idx) ? (GWResourceKind)mList[idx].mKind : GWResourceKind::UNKNOWN; }
	uint32_t get_kind_count(GWResourceKind kind) const {
		int count = 0;
		for (uint32_t i = 0; i < mNum; ++i) {
			if (mList[i].mKind == (uint32_t)kind) { ++count; }
		}
		return count;
	}

	static GWCatalog* load(const std::string& path) {
		GWCatalog* pCat = nullptr;
		GWResource* pRsrc = GWResource::load(path, GW_RSRC_ID("GWCatalog"));
		if (pRsrc) {
			pCat = reinterpret_cast<GWCatalog*>(pRsrc);
		}
		return pCat;
	}
};

namespace GWResourceUtil {
/*
	struct GPUIfc {
		void (*prepareModel)(GWModelResource* pMdl);
		void (*releaseModel)(GWModelResource* pMdl);
		void (*prepareImage)(GWImage* pImg);
		void (*releaseImage)(GWImage* pImg);

		void reset() {
			prepareModel = nullptr;
			releaseModel = nullptr;
			prepareImage = nullptr;
			releaseImage = nullptr;
		}
	};
	GPUIfc* get_gpu_ifc();
	void set_gpu_ifc(GPUIfc* pIfc);
*/
	typedef void (*ModelBindFunc)(GWModelResource* pMdlRsc);
	typedef void (*ModelUnbindFunc)(GWModelResource* pMdlRsc);

	void set_model_binding(ModelBindFunc* pBind, ModelUnbindFunc* pUnbind);
	void bind(GWModelResource* pMdlRsc);
	void unbind(GWModelResource* pMdlRsc);

	typedef void (*ImageBindFunc)(GWImage* pImg);
	typedef void (*ImageUnbindFunc)(GWImage* pImg);

	void set_image_binding(ImageBindFunc* pBind, ImageUnbindFunc* pUnbind);
	void bind(GWImage* pImg);
	void unbind(GWImage* pImg);

	const char* name_from_path(const char* pPath, char sep = '/');
	const char* get_kind_string(GWResourceKind kind);
	void set_fallback_bundle(GWBundle* pBdl);
	GWBundle* get_fallback_bundle();
}

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

	void unbind_models();
	void purge_models();

	void unbind_images();
	void purge_images();

	void purge_motions();
	void purge_colli_data();
	void unbind_all();

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