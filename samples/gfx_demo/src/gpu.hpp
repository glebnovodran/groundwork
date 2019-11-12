enum GPUTexUnit {
	TEXUNIT_BASE = 0,
	TEXUNIT_NORM,
	TEXUNIT_SPEC,
	TEXUNIT_SHADOW
};

struct GPUAttrLink {
	void reset() { memset(this, 0xFF, sizeof(*this)); }
};

struct GPUParamLink {
	void reset() { memset(this, 0xFF, sizeof(*this)); }
};

struct GPUSampLink {
	void reset() { memset(this, 0xFF, sizeof(*this)); }
};

struct GPUProg {
	GLuint mProgId;
	GPUAttrLink mAttrLink;
	GPUParamLink mParamLink;
	GPUSampLink mSampLink;
	const char* mpVertName;
	const char* mpFragName;
	char mStrBuf[128];

	GPUProg() : mProgId(0), mpVertName(nullptr), mpFragName(nullptr) {
		mAttrLink.reset();
		mParamLink.reset();
		mSampLink.reset();
	}

	bool valid() const { return !!mProgId; }

	void create(const char* pDescr);
	void init();
	void reset();
	void use();

};

struct GPUTex {
	GLuint mHandle;
	int mWidth;
	int mHeight;
	uint32_t* mpData;

	GPUTex() : mHandle(0), mWidth(0), mHeight(0), mpData(nullptr) {}

	~GPUTex() {
		destroy();
	}

	void create(const GWImage& img, bool nonlin);
	void destroy();
	GLuint get_handle();
	void reset();
};

class GPUModel {
public:
	struct BBox {
		GWVectorF mMin;
		GWVectorF mMax;

		GWVectorF size() const { return mMax - mMin; }

		void zero() {
			mMin.fill(0.0f);
			mMax.fill(0.0f);
		}
	};

	struct Vtx {};

	struct Mtl {};

	struct Batch {};

	struct Skel {};

	struct DrawCtx {};

	GPUModel() {}

	~GPUModel() {
		destroy();
	}

	void create(GWModelResource& mdr);
	void destroy();
	void init();
	void reset();

};