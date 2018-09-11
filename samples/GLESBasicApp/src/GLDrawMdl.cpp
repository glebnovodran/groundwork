/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#define DYNAMICGLES_NO_NAMESPACE
#define DYNAMICEGL_NO_NAMESPACE

#include <DynamicGles.h>
#include "groundwork.hpp"
#include "GLDraw.hpp"

namespace GLDraw {
	GWNamedObjList<Model> s_models;
	GWNamedObjList<Texture> s_textures;

	void Material::init(const Model* pMdl, const GWModelResource::Material& mtlRsc, uint32_t id) {
		mpMdl = pMdl;
		mId = id;
		const char* pMtlName = pMdl->get_str(mtlRsc.mPathOffs);
		mItem.set_name_val(pMtlName, this);
		const char* pBaseName = GWResourceUtil::name_from_path(pMdl->get_str(mtlRsc.mBaseMapPathOffs));
		mFlags = mtlRsc.mFlags;
		mParams.mBaseColor = mtlRsc.mBaseColor;
		mParams.mSpecColor = mtlRsc.mSpecColor;
		mParams.mRoughness = mtlRsc.mRoughness;
		mParams.mFresnel = mtlRsc.mFresnel;
		mParams.mBumpScale = mtlRsc.mBumpScale;
		mIdx = mtlRsc.mIdx;
	}

	Model* create_model(GWModelResource& mdlRsc, const char* pName) {
		uint32_t vtxNum = mdlRsc.mNumPnt;
		if (vtxNum == 0) { return nullptr; }
		uint32_t triNum = mdlRsc.mNumTri;
		if (triNum == 0) { return nullptr; }
		uint32_t mtlNum = mdlRsc.mNumMtl;
		if (mtlNum == 0) {
			// TODO: set default material and continue
			return nullptr;
		}
		uint32_t batchNum = mtlNum;

		uint32_t id[3];
		glGenBuffers(3, id);
		if ((0 == id[0]) || (0 == id[1]) || (0 == id[1])) {
			gl_error();
			return nullptr;
		}

		Model* pMdl = new Model();
		pMdl->mNumPnt = vtxNum;
		pMdl->mNumTri = triNum;
		pMdl->mNumMtl = mtlNum;
		pMdl->mNumBatch = mtlNum; // == mNumMtl for the time being
		pMdl->mBuffIdVtx = id[0];
		pMdl->mBuffIdIdx16 = id[1];
		pMdl->mBuffIdIdx32 = id[2];

		const char* pStrs = mdlRsc.get_str();
		uint32_t strsSize = mdlRsc.mStrsSize;
		pMdl->mpStrs = new char[strsSize];
		std::copy_n(pStrs, strsSize, pMdl->mpStrs);
		pMdl->mStrsSize = strsSize;
		pMdl->mPathOffs = mdlRsc.mPathOffs;

		Vertex* pVtx = new Vertex[vtxNum];
		for (uint32_t i = 0; i < vtxNum; i++) {
			pVtx->mPos = mdlRsc.get_pnt(i);
			GWModelResource::Attr* pAttr = mdlRsc.get_attr(i);
			pVtx->mNrmTgtEnc = pAttr->mNrmTgtEnc;
			pVtx->mColor = pAttr->mColor;
			pVtx->mTex = pAttr->mTex;
		}
		glBindBuffer(GL_ARRAY_BUFFER, pMdl->mBuffIdVtx);
		glBufferData(GL_ARRAY_BUFFER, vtxNum * sizeof(Vertex), pVtx, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		delete[] pVtx;


		uint16_t* pIdx16 = mdlRsc.get_idx16();
		if (pIdx16 == nullptr) {
			pMdl->mBuffIdIdx16 = 0;
		} else {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMdl->mBuffIdIdx16);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, mdlRsc.mNumIdx16, pIdx16, GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		uint32_t* pIdx32 = mdlRsc.get_idx32();
		if (pIdx32 == nullptr) {
			pMdl->mBuffIdIdx32 = 0;
		} else {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMdl->mBuffIdIdx32);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, mdlRsc.mNumIdx32, pIdx32, GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
		
		Material* pMtls = new Material[mtlNum];
		Batch* pBatch = new Batch[batchNum];

		for (uint32_t i = 0; i < mdlRsc.mNumMtl; ++i) {
			GWModelResource::Material* pMtlRsc = mdlRsc.get_mtl(i);
			pMtls[i].init(pMdl, *pMtlRsc, i);
			pMdl->mMaterials.add(&pMtls[i].mItem);
			pBatch[i].mpMdl = pMdl;
			pBatch[i].mMtlId = i;
		}
		pMdl->mpBatch = pBatch;
		pMdl->mpMtls = pMtls;
		char* pMdlName = const_cast<char*>((pName == nullptr) ? pMdl->get_resource_path() : pName);

		pMdl->mItem.set_name_val(pMdlName, pMdl);
		s_models.add(&pMdl->mItem);
		return pMdl;
	}

	Model* find_model(const std::string& name) {
		ModelItem* pItem = s_models.find_first(name.c_str());
		return pItem == nullptr ? nullptr : pItem->mpVal;
	}

	void free_model(Model* pMdl) {
		s_models.remove(&pMdl->mItem);
		delete pMdl;
	}

	Model::~Model() {
		if (0 != mBuffIdVtx) { glDeleteBuffers(1, &mBuffIdVtx); }
		if (0 != mBuffIdIdx16) { glDeleteBuffers(1, &mBuffIdIdx16); }
		if (0 != mBuffIdIdx32) { glDeleteBuffers(1, &mBuffIdIdx32); }

		if (mpStrs != nullptr) { delete[] mpStrs; }
		if (mpBatch != nullptr) { delete[] mpBatch; }
		if (mpMtls != nullptr) { delete[] mpMtls; }
	}

	Texture* create_texture(const GWImage& img, const char* pName) {
		Texture* pTex = nullptr;
		uint32_t texId = 0;
		glGenTextures(1, &texId);
		if (0 != texId) {
			pTex = new Texture();
			int w = img.get_width();
			int h = img.get_height();
			int n = h * w;

			pTex->mId = texId;
			pTex->mWidth = w;
			pTex->mHeight = h;

			const GWColorF* pClr = img.get_pixels();
			const float* pClrBuf = reinterpret_cast<const float*>(img.get_pixels());
			if (img.is_hdr()) {
				GWHalf4* pEnc = new GWHalf4[n];
				// RFU: Don't encode to half-float through glTexImage2D
				GWBase::float_to_half((uint16_t*)pEnc, pClrBuf, n * 4);

				glBindTexture(GL_TEXTURE_2D, texId);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_HALF_FLOAT, pEnc);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glGenerateMipmap(texId);

				delete[] pEnc;
			} else {
			}
			pTex->mItem.set_name_val(pName, pTex);
			s_textures.add(&pTex->mItem);
		} else {
			gl_error();
		}
		return pTex;
	}

	Texture* create_texture(const char* pPath, const char* pName) {
		Texture* pTex = nullptr;
		GWImage* pImg = GWImage::read_dds(pPath);
		if (pImg != nullptr) {
			pTex = create_texture(*pImg, pName);
			GWImage::free(pImg);
		}

		return pTex;
	}

	Texture* find_texture(const char* pName) {
		TextureItem* pItem = s_textures.find_first(pName);
		return pItem == nullptr ? nullptr : pItem->mpVal;
	}

	void free_texture(Texture* pTex) {
		if (0 != pTex->mId) {
			glDeleteTextures(1, &pTex->mId);
		}
		s_textures.remove(&pTex->mItem);
		delete pTex;
	}
}
