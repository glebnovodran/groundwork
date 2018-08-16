/*
* Author: Gleb Novodran <novodran@gmail.com>
*/

#define GW_RSRC_SIG "rsrc:"
#define GW_RSRC_ID(_name) GW_RSRC_SIG##_name

struct GWResource {
	/* +00*/ char mSignature[0x10];
	/* +10*/ uint32_t mVersion;
	/* +14*/ uint32_t mDataSize;
	/* +18*/ uint32_t mStrsTop;
	/* +1c*/ uint32_t mStrsSize;

	const char* get_str(uint32_t offs) const {
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
	static void unload(GWResource* pRsrc) {
		if (pRsrc) {
			delete[] reinterpret_cast<char*>(pRsrc);
		}
	}
};

struct GWModelResource : public GWResource {
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
		typedef uint16_t HalfVec4[4];

		HalfVec4 mNrmTgtEnc;
		HalfVec4 mColor;
		HalfVec4 mTex;

		GWVectorF get_normal() const {
			GWVectorF vec;
			float oct[2];
			GWBase::half_to_float(oct, &mNrmTgtEnc[0], 2);
			GWBase::oct_to_vec(oct[0], oct[1], vec.x, vec.y, vec.z);
		}

		GWVectorF get_tangent() const {
			GWVectorF vec;
			float oct[2];
			GWBase::half_to_float(oct, &mNrmTgtEnc[2], 2);
			GWBase::oct_to_vec(oct[0], oct[1], vec.x, vec.y, vec.z);
		}

		GWColorF get_color() const {
			GWColorF clr;
			GWBase::half_to_float(&clr.elems[0], &mColor[0], 4);
			return clr;
		}

		GWColorTuple3f get_rgb() const {
			GWColorTuple3f rgb;
			GWBase::half_to_float(&rgb.elems[0], &mColor[0], 3);
			return rgb;
		}

		GWTuple4f get_tex() const {
			GWTuple4f tex;
			GWBase::half_to_float(&tex.elems[0], &mTex[0], 4);
			return tex;
		}

		GWTuple2f get_uv() const {
			GWTuple2f uv;
			GWBase::half_to_float(&uv.elems[0], &mTex[0], 2);
			return uv;
		}

		GWTuple2f get_uv2() const {
			GWTuple2f uv2;
			GWBase::half_to_float(&uv2.elems[0], &mTex[2], 2);
			return uv2;
		}
	};

	struct Material {
		uint32_t mPathOffs;
		uint32_t mFlags;
		uint32_t mBaseMapPathOffs;
		uint32_t mExtParamsOffs;
		int32_t mIdxOrg;
		int32_t mNumTris;
		int32_t mMinIdx;
		int32_t mMaxIdx;
		GWColorTuple3f mBaseColor;
		GWColorTuple3f mSpecColor;
		float mRoughness;
		float mFresnel;
		float mBumpScale;

		bool get_flag(int idx) const { return !!(mFlags & (1 << idx)); }
		bool is_idx16() const { return (mMaxIdx - mMinIdx) < (1 << 16); }
		bool is_double_sided() const { return get_flag(0); }
		bool is_semi_transparent() const { return get_flag(1); }
		bool get_tangent_flip_flag() const { return get_flag(2); }
		bool get_bitangent_flip_flag() const { return get_flag(3); }
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
				GWBase::half_to_float(&alpha, &pAttr->mColor[3], 1);
			}
		}
		return alpha;
	}

	float get_pnt_ao(uint32_t idx) {
		float occl = 1.0f;
		if (valid_ao()) {
			Attr* pAttr = get_attr(idx);
			if (pAttr) {
				GWBase::half_to_float(&occl, &pAttr->mColor[3], 1);
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


	GWTransformF get_skel_node_local_mtx(uint32_t idx) {
		GWTransformF lm;
		lm.set_identity();
		if (check_skel_node_idx(idx)) {
			lm = reinterpret_cast<GWTransformF*>(get_ptr(mOffsSkel))[idx];
		}
		return lm;
	}

	GWTransformF calc_skel_node_world_mtx(uint32_t idx, const GWTransformF* pLM = nullptr, GWTransformF* pParentWM = nullptr);

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
	GWTuple4i get_pnt_skin_joints(int pntIdx);
	GWTuple4f get_pnt_skin_weights(int pntIdx);

	int get_pnt_skin_joints_count(int pntIdx);

	static GWModelResource* load(const std::string& path);
};
