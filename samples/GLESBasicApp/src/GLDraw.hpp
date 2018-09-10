/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

struct GLDrawCfg {
	int width;
	int height;
};

namespace GLDraw {
	class Texture;
	class Model;
	class Material;

	static const uint32_t NONE = (uint32_t)-1;
	bool gl_error();

	Texture* create_texture(const GWImage& img, const char* pName = nullptr);
	Texture* create_texture(const char* pPath, const char* pName = nullptr);
	Texture* find_texture(const char* pName);
	void free_texture(Texture* pTex);

	Model* create_model(GWModelResource& mdlRsrc, const char* pName = nullptr);
	Model* find_model(const std::string& name);
	void free_model(Model* pMdl);

	void init(const GLDrawCfg& cfg);
	void reset();
	void loop(void(*pLoop)());

	void set_view(const GWVectorF& pos, const GWVectorF& tgt, const GWVectorF& up = GWVectorF(0.0f, 1.0f, 0.0f));
	void set_degreesFOVY(float deg);
	void set_view_range(float znear, float zfar);

	void begin();
	void end();

	typedef GWListItem<Model> ModelItem;
	typedef GWListItem<Material> MaterialItem;
	typedef GWListItem<Texture> TextureItem;

	struct Vertex {
		GWVectorF mPos;
		GWHalf4  mNrmTgtEnc;
		GWHalf4  mColor;
		GWHalf4  mTex;
	};

	struct Batch {
		Model* mpMdl;
		uint16_t mMtlId;
	};

	class Model {
	protected:
		ModelItem mItem;
		GWNamedObjList<Material> mMaterials;
		Material* mpMtls;
		Batch* mpBatch;
		char* mpStrs;
		uint32_t mStrsSize;

		uint32_t mPathOffs;
		uint32_t mNumPnt;
		uint32_t mNumTri;
		uint32_t mNumMtl;
		uint32_t mNumBatch;

		uint32_t mBuffIdVtx;
		uint32_t mBuffIdIdx16;
		uint32_t mBuffIdIdx32;
	private:
		Model() : mpStrs(nullptr), mStrsSize(0) {}

	public:
		~Model();

		const char* get_str(uint32_t offs) const{
			return mpStrs + offs;
		}
		const char* get_resource_path() const { return get_str(mPathOffs); }
		const char* get_name() const { return mItem.mpName; }

		uint32_t find_material(std::string& name) const;
		Material* get_material(uint32_t id) const;

		friend Model* GLDraw::create_model(GWModelResource& mdlRsc, const char* pName);
		friend void GLDraw::free_model(Model* pMdl);
	};

	class Material {
	protected:
		MaterialItem mItem;
		const Model* mpMdl;

		struct {
			GWColorTuple3f mBaseColor;
			GWColorTuple3f mSpecColor;
			float mRoughness;
			float mFresnel;
			float mBumpScale;
		} mParams;
		GWModelResource::MtlFlags mFlags;

		GWModelResource::IdxInfo mIdx;
		uint32_t mId;
		uint32_t mNameOffs;
		uint32_t mBaseMapOffs;
		uint32_t mUBId;
	public:
		void init(const Model* pMdl, const GWModelResource::Material& mtlRsc, uint32_t id);
		void set_base_color(float r, float g, float b);
		void set_base_color(const GWColorF& color);
		void set_roughness(float r);
		void set_fresnel(float f);
		void set_bump_scale(float s);

		void fix_textures();

		friend Model* GLDraw::create_model(GWModelResource& mdlRsc, const char* pName);
	};

	class Texture {
	protected:
		TextureItem mItem;
	public:
		uint32_t mId;
		uint32_t mWidth;
		uint32_t mHeight;

		Texture() : mId(0), mWidth(0), mHeight(0) {}

		const char* get_name() const { return mItem.mpName; }

		friend Texture* GLDraw::create_texture(const GWImage& img, const char* pName);
		friend void GLDraw::free_texture(Texture* pTex);
	};
}
