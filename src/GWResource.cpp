/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <fstream>
#include <cstddef>
#include <vector>
#include "groundwork.hpp"

namespace GWResourceUtil {

	static GWBundle* s_pFallbackBundle = nullptr;
	static ModelBindFunc s_modelBind = nullptr;
	static ModelUnbindFunc s_modelUnbind = nullptr;
	static ImageBindFunc s_imageBind = nullptr;
	static ImageUnbindFunc s_imageUnbind = nullptr;

	void set_model_binding(ModelBindFunc pBind, ModelUnbindFunc pUnbind) {
		s_modelBind = pBind;
		s_modelUnbind = pUnbind;
	}
	void bind(GWModelResource* pMdlRsc) {
		if (s_modelBind) { (s_modelBind)(pMdlRsc); }
	}
	void unbind(GWModelResource* pMdlRsc) {
		if (s_modelUnbind) { (s_modelUnbind)(pMdlRsc); }
	}

	void set_image_binding(ImageBindFunc pBind, ImageUnbindFunc pUnbind) {
		s_imageBind = pBind;
		s_imageUnbind = pUnbind;
	}
	void bind(GWImage* pImage) {
		if (s_imageBind) { (s_imageBind)(pImage); }
	}
	void unbind(GWImage* pImage) {
		if (s_imageUnbind) { (s_imageUnbind)(pImage); }
	}

	const char* name_from_path(const char* pPath, char sep) {
		const char* pName = nullptr;
		if (pPath) {
			const char* p = pPath + ::strlen(pPath);
			for (; --p >= pPath;) {
				if (*p == sep) {
					pName = p + 1;
					break;
				}
			}
			if (!pName) pName = pPath;
		}
		return pName;
	}

	const char* get_kind_string(GWResourceKind kind) {
		const char* pStr = "UNKNOWN";
		switch (kind) {
		case GWResourceKind::CATALOG: pStr = "Catalogue"; break;
		case GWResourceKind::MODEL: pStr = "Model"; break;
		case GWResourceKind::COL_DATA: pStr = "Collision geo"; break;
		case GWResourceKind::DDS: pStr = "DDS"; break;
		case GWResourceKind::TDMOT: pStr = "TDMotion"; break;
		case GWResourceKind::TDGEO: pStr = "TDGeometry"; break;
		default: break;
		}
		return pStr;
	}

	void set_fallback_bundle(GWBundle* pBdl) {
		s_pFallbackBundle = pBdl;
	}

	GWBundle* get_fallback_bundle() {
		return s_pFallbackBundle;
	}

} // namespace GWResourceUtil

void write_py_mtx(std::ostream& os, const GWTransformF& xform) {
	const float* pData = xform.as_tptr();
	os << "[";
	for (int i = 0; i < 4; ++i) {
		os << "[";
		for (int j = 0; j < 4; ++j, ++pData) {
			os << (*pData);
			if (j < 3) os << ", ";
		}
		os << "]";
		if (i < 3) os << ", ";
	}
	os << "]";
}

void GWResource::unload(GWResource* pRsrc) {
	if (pRsrc) {
		if (pRsrc->binding_memory_allocated()) {
			GWSys::dbg_msg("Warning: Unloading a resource that has an allocated binding memory");
		}
		delete[] reinterpret_cast<char*>(pRsrc);
	}
}

GWResource* GWResource::load(const std::string& path, const char* pSig) {
	char sig[0x10];
	char* pBuf = nullptr;
	size_t fsize = 0;
	FILE* pFile = nullptr;
	const char* pMode = "rb";
#if defined(_MSC_VER)
	fopen_s(&pFile, path.c_str(), pMode);
#else
	pFile = fopen(path.c_str(), pMode);
#endif
	if (pFile) {
		if (fseek(pFile, 0, SEEK_END) == 0) {
			fsize = ftell(pFile);
		}
		fseek(pFile, 0, SEEK_SET);
		if (fsize > 0x10) {
			fread(sig, 1, 0x10, pFile);
			if (::memcmp(sig, GW_RSRC_SIG, sizeof(GW_RSRC_SIG) - 1) == 0) {
				size_t size = fsize + sizeof(Binding); // unaligned
				fseek(pFile, 0, SEEK_SET);
				pBuf = new char[size];
				if (pBuf) {
					fread(pBuf, 1, fsize, pFile);
					uint8_t* pBinding = GWBase::incr_ptr(pBuf, fsize);
					std::fill_n(pBinding, sizeof(Binding),0);
				}
			}
		}
		fclose(pFile);
	}
	return reinterpret_cast<GWResource*>(pBuf);
}

const char* GWModelResource::get_mtl_name(uint32_t idx) {
	return GWResourceUtil::name_from_path(get_mtl_path(idx));
}

GWTransformF GWModelResource::calc_skel_node_world_xform(uint32_t idx, const GWTransformF* pLM, GWTransformF* pParentWM) {
	GWTransformF nodeWM;
	GWTransformF parentWM;
	nodeWM.set_identity();
	parentWM.set_identity();

	if (has_skel()) {
		if (!pLM) {
			pLM = reinterpret_cast<const GWTransformF*>(get_ptr(mOffsSkel));
		}
		if (check_skel_node_idx(idx)) {
			nodeWM = pLM[idx];
		}
		idx = get_skel_node_parent_idx(idx);
		while (check_skel_node_idx(idx)) {
			parentWM = GWXform::concatenate(parentWM, pLM[idx]);
			idx = get_skel_node_parent_idx(idx);
		}
		nodeWM = GWXform::concatenate(nodeWM, parentWM);
	}
	if (pParentWM) {
		*pParentWM = parentWM;
	}
	return nodeWM;
}

uint32_t GWModelResource::find_skel_node_skin_idx(uint32_t skelIdx) {
	uint32_t skinIdx = NONE;
	if (check_skel_node_idx(skelIdx)) {
		if (has_skin()) {
			uint32_t* pSkinToSkel = get_skin_to_skel_map();
			for (uint32_t i = 0; i < mNumSkinNodes; ++i) {
				if (skelIdx == pSkinToSkel[i]) {
					skinIdx = i;
					break;
				}
			}
		}
	}
	return skinIdx;
}

const char* GWModelResource::get_skin_node_name(uint32_t skinIdx) {
	const char* pName = nullptr;
	if (has_skin() && check_skel_node_idx(skinIdx)) {
		uint32_t skinNameOffs = reinterpret_cast<uint32_t*>(get_ptr(mOffsSkin))[skinIdx];
		pName = get_str(skinNameOffs);
	}
	return pName;
}

GWTuple4u GWModelResource::get_pnt_skin_joints(uint32_t pntIdx) {
	GWTuple4u jnt;
	GWTuple::fill(jnt, 0);
	if (has_skin() && check_pnt_idx(pntIdx)) {
		void* pData = get_skin_data();
		bool byteIdxFlg = mNumSkinNodes <= (1 << 8);
		size_t wgtSize = byteIdxFlg ? 4 + 4 : (4 * 2) + 4;
		pData = reinterpret_cast<char*>(pData) + (pntIdx * wgtSize);
		if (byteIdxFlg) {
			uint8_t* pJnt = reinterpret_cast<uint8_t*>(pData);
			for (int j = 0; j < 4; ++j) {
				jnt[j] = pJnt[j];
			}
		} else {
			uint16_t* pJnt = reinterpret_cast<uint16_t*>(pData);
			for (int j = 0; j < 4; ++j) {
				jnt[j] = pJnt[j];
			}
		}
	}
	return jnt;
}

GWTuple4f GWModelResource::get_pnt_skin_weights(uint32_t pntIdx) {
	GWTuple4f wgt;
	GWTuple::fill(wgt, 0.0f);
	if (has_skin() && check_pnt_idx(pntIdx)) {
		void* pData = get_skin_data();
		bool byteIdxFlg = mNumSkinNodes <= (1 << 8);
		size_t wgtSize = byteIdxFlg ? 4 + 4 : (4 * 2) + 4;
		pData = reinterpret_cast<char*>(pData) + (pntIdx * wgtSize) + (byteIdxFlg ? 4 : 4 * 2);
		uint8_t* pWgt = reinterpret_cast<uint8_t*>(pData);
		for (int j = 0; j < 4; ++j) {
			wgt[j] = float(pWgt[j]);
		}
		GWTuple::scl(wgt, 1.0f / 255);
	}
	return wgt;
}

uint32_t GWModelResource::get_pnt_skin_joints_count(uint32_t pntIdx) {
	int numJnt = 0;
	if (has_skin() && check_pnt_idx(pntIdx)) {
		void* pData = get_skin_data();
		bool byteIdxFlg = mNumSkinNodes <= (1 << 8);
		size_t wgtSize = byteIdxFlg ? 4 + 4 : (4 * 2) + 4;
		pData = reinterpret_cast<char*>(pData) + (pntIdx * wgtSize) + (byteIdxFlg ? 4 : 4 * 2);
		uint8_t* pWgt = reinterpret_cast<uint8_t*>(pData);
		for (int j = 0; j < 4; ++j) {
			if (pWgt[j] == 0) break;
			++numJnt;
		}
	}
	return numJnt;
}
GWSphereF GWModelResource::calc_skin_node_sphere_of_influence(uint32_t skinIdx, GWVectorF* pMem) {
	GWSphereF sph(0.0f, 0.0f, 0.0f, 0.0f);

	if (check_skin_node_idx(skinIdx)) {
		GWVectorF* pMdlPts = reinterpret_cast<GWVectorF*>(get_pnt_ptr(0));
		GWVectorF* pPts = (pMem == nullptr) ? new GWVectorF[mNumPnt] : pMem;

		uint32_t k = 0;
		for (uint32_t i = 0; i < mNumPnt; ++i) {
			uint32_t numJnt = get_pnt_skin_joints_count(i);
			GWTuple4u ptJnts = get_pnt_skin_joints(i);
			for (uint32_t j = 0; j < numJnt; ++j) {
				if (ptJnts[j] == skinIdx) {
					pPts[k] = pMdlPts[i];
					++k;
					break;
				}
			}
		}

		sph.ritter(pPts, k);
		if (pMem == nullptr) { delete[] pPts; }
	}
	return sph;
}

GWSphereF* GWModelResource::calc_skin_spheres_of_influence() {
	GWSphereF* pSph = nullptr;
	if (has_skin()) {
		GWVectorF* pMem = new GWVectorF[mNumPnt];
		pSph = new GWSphereF[mNumSkinNodes];
		for (uint32_t i = 0; i < mNumSkinNodes; ++i) {
			if (is_skel_node_skin_deformer(i)) {
				pSph[i] = calc_skin_node_sphere_of_influence(i, pMem);
			} else {
				pSph[i].set_zero();
			}
		}
		delete[] pMem;
	}
	return pSph;
}

GWModelResource* GWModelResource::load(const std::string& path) {
	GWModelResource* pMdr = nullptr;
	GWResource* pRsrc = GWResource::load(path, GW_RSRC_ID("GWModel"));
	if (pRsrc) {
		pMdr = reinterpret_cast<GWModelResource*>(pRsrc);
		GWSys::dbg_msg("+ model resource: %s\n", pMdr->get_path());
	}
	return pMdr;
}

void GWModelResource::write_geo(std::ostream& os) {
	using namespace std;

	int numAttr = 0;

	if (valid_nrm()) numAttr++;
	if (valid_tng()) numAttr++;
	if (valid_rgb()) numAttr++;
	if (valid_uv()) numAttr++;
	if (valid_ao()) numAttr++;

	os << "PGEOMETRY V5" << endl;
	os << "NPoints " << mNumPnt << " NPrims " << mNumTri << endl;
	os << "NPointGroups 0 NPrimGroups " << mNumMtl << endl;
	os << "NPointAttrib " << numAttr << " NVertexAttrib 0 NPrimAttrib 0 NAttrib 0" << endl;
	if (numAttr > 0) {
		os << "PointAttrib" << endl;
	}
	if (valid_nrm()) {
		os << "N 3 vector 0 0 0" << endl;
	}
	if (valid_tng()) {
		os << "tangentu 3 vector 0 0 0" << endl;
	}
	if (valid_rgb()) {
		os << "Cd 3 float 1 1 1" << endl;
	}
	if (valid_uv()) {
		os << "uv 3 float 0 0 1" << endl;
	}
	if (valid_ao()) {
		os << "AO 1 float 1" << endl;
	}

	for (uint32_t i = 0; i < mNumPnt; ++i) {
		Attr* pAttr = get_attr(i);
		GWVectorF pnt = get_pnt(i);
		os << pnt.x << " " << pnt.y << " " << pnt.z << " 1";
		if (numAttr > 0) {
			os << " (";
			if (valid_nrm()) {
				GWVectorF nrm = pAttr->get_normal();
				os << " " << nrm.x << " " << nrm.y << " " << nrm.z << " ";
			}
			if (valid_tng()) {
				GWVectorF tng = pAttr->get_tangent();
				os << " " << tng.x << " " << tng.y << " " << tng.z << " ";
			}
			if (valid_rgb()) {
				GWColorTuple3f rgb = pAttr->get_rgb();
				os << " " << rgb.r << " " << rgb.g << " " << rgb.b << " ";
			}
			if (valid_uv()) {
				GWTuple2f uv = pAttr->get_uv();
				os << " " << uv.x << " " << 1.0f - uv.y << " 1 ";
			}
			if (valid_ao()) {
				os << " " << get_pnt_ao(i) << " ";
			}
			os << ")";
		}
		os << endl;
	}

	os << "Run " << mNumTri << " Poly" << endl;
	for (uint32_t i = 0; i < mNumMtl; ++i) {
		Material* pMtl = get_mtl(i);
		for (uint32_t j = 0; j < pMtl->mIdx.mNumTri; ++j) {
			uint32_t idx[3];
			if (pMtl->mIdx.is_idx16()) {
				uint16_t* pIdx16 = reinterpret_cast<uint16_t*>(get_ptr(mOffsIdx16)) + pMtl->mIdx.mOrg;
				for (int k = 0; k < 3; ++k) {
					idx[k] = pIdx16[(j * 3) + k];
				}
			} else {
				uint32_t* pIdx32 = reinterpret_cast<uint32_t*>(get_ptr(mOffsIdx32)) + pMtl->mIdx.mOrg;
				for (int k = 0; k < 3; ++k) {
					idx[k] = pIdx32[(j * 3) + k];
				}
			}
			for (int k = 0; k < 3; ++k) {
				idx[k] += pMtl->mIdx.mMin;
			}
			os << " 3 < " << idx[0] << " " << idx[1] << " " << idx[2] << endl;
		}
	}

	uint32_t triOrg = 0;
	for (uint32_t i = 0; i < mNumMtl; ++i) {
		Material* pMtl = get_mtl(i);
		const char* pMtlName = get_mtl_name(i);
		os << pMtlName << " unordered" << endl;
		os << mNumTri << " ";
		int cnt = 0;
		for (uint32_t j = 0; j < mNumTri; ++j) {
			char cflg = j >= triOrg && j < triOrg + pMtl->mIdx.mNumTri ? '1' : '0';
			os << cflg;
			++cnt;
			if (cnt > 64) {
				os << endl;
				cnt = 0;
			}
		}
		triOrg += pMtl->mIdx.mNumTri;
		os << endl;
	}
	os << "beginExtra" << endl;
	os << "endExtra" << endl;
}

void GWModelResource::write_skel(std::ostream& os, const char* pBase) {
	using namespace std;
	if (!has_skel()) return;
	if (!pBase) {
		pBase = "/obj";
	}
	int n = mNumSkelNodes;
	GWSphereF* pSph = calc_skin_spheres_of_influence();
	for (int i = 0; i < n; ++i) {
		const char* pNodeName = get_skel_node_name(i);
		if (pNodeName) {
			os << "# "; write_py_mtx(os, calc_skel_node_world_xform(i)); os << endl;
			os << "nd = hou.node('" << pBase << "').createNode('null', '" << pNodeName << "')" << endl;
			GWTransformF lm = get_skel_node_local_mtx(i);
			os << "nd.setParmTransform(hou.Matrix4(";
			write_py_mtx(os, lm);
			os << "))" << endl;
			bool skinFlg = is_skel_node_skin_deformer(i);
			os << "nd.setParms({'geoscale':0.01,'controltype':1})" << endl;
			os << "nd.setUserData('nodeshape', '" << (skinFlg ? "bone" : "rect") << "')" << endl;
			if (skinFlg) {
				GWSphereF sph= pSph[i];
				os << "# " << pNodeName << " skin SOI: " << sph.c.x << ", " << sph.c.y << ", " << sph.c.z << ", " << sph.r << endl;
				os << "cr = nd.createNode('cregion', 'cregion')" << endl;
				os << "cr.setParms({'squashx':0.0001,'squashy':0.0001,'squashz':0.0001})" << endl;
			}
		}
	}
	for (int i = 0; i < n; ++i) {
		uint32_t parentIdx = get_skel_node_parent_idx(i);
		if (check_skel_node_idx(parentIdx)) {
			const char* pNodeName = get_skel_node_name(i);
			const char* pParentName = get_skel_node_name(parentIdx);
			os << "hou.node('" << pBase << "/" << pNodeName << "').setFirstInput(hou.node('" << pBase << "/" << pParentName << "'))" << endl;
		}
	}

	if (pSph != nullptr) { delete[] pSph; }
}


static void rand_pol_colors(std::vector<GWColorF>& c, const int n) {
	GWBase::Random rnd;
	const int mask = 0x3F;
	const int base = 0x90;
	for (int i = 0; i < n; ++i) {
		GWColorF rc;
		for (int j = 0; j < 3; ++j) {
			int rv = base + int(rnd.u64() & mask);
			rc.elems[j] = float(rv);
		}
		rc.a = 255.0f;
		rc.scl(1.0f / rc.a);
		c.push_back(rc);
	}
}

int GWCollisionResource::calc_num_tris() {
	int ntris = 0;
	Poly* pPols = get_pols_top();
	if (pPols) {
		for (int i = 0; i < mNumPol; ++i) {
			int nvtx = pPols[i].mNumVtx;
			ntris += nvtx - 2;
		}
	}
	return ntris;
}

bool GWCollisionResource::get_poly_tri(GWVectorF vtx[3], int polIdx, int triIdx) {
	bool res = false;
	GWVectorF* pPnts = get_pnts_top();
	Poly* pPols = get_pols_top();
	int32_t* pIdx = get_idx_top();
	if (pPnts && pPols && pIdx && check_poly_idx(polIdx)) {
		int pntIdx[3];
		Poly* pPol = &pPols[polIdx];
		int nvtx = pPol->mNumVtx;
		int ntri = nvtx - 2;
		if (triIdx >= 0 && triIdx < ntri) {
			if (nvtx > 3) {
				int32_t* pTris = get_tris_top();
				if (pTris) {
					for (int i = 0; i < 3; ++i) {
						pntIdx[i] = pTris[pPol->mOffsTris + triIdx*3 + i];
					}
					for (int i = 0; i < 3; ++i) {
						pntIdx[i] = pIdx[pPol->mOffsIdx + pntIdx[i]];
					}
				}
			} else {
				for (int i = 0; i < 3; ++i) {
					pntIdx[i] = pIdx[pPol->mOffsIdx + i];
				}
			}
			res = true;
		}
	}
	return res;
}

int GWCollisionResource::get_poly_num_tris(int polIdx) {
	int nvtx = 0;
	Poly* pPols = get_pols_top();
	if (pPols && check_poly_idx(polIdx)) {
		nvtx = pPols->mNumVtx - 2;
	}
	return nvtx;
}

static GWVectorF calc_tri_normal(GWVectorF v[3]) {
	GWVectorF nrm = GWVector::cross(v[0] - v[1], v[2] - v[1]);
	nrm.normalize();
	return nrm;
}

int GWCollisionResource::for_all_tris(TriFunc& func, bool withNormals) {
	int triCount = 0;
	Poly* pPols = get_pols_top();
	if (pPols) {
		GWVectorF vtx[3];
		GWVectorF nrm(0.0f);
		for (int i = 0; i < mNumPol; ++i) {
			int ntri = get_poly_num_tris(i);
			for (int j = 0; j < ntri; ++j) {
				if (get_poly_tri(vtx, i, j)) {
					if (withNormals) {
						nrm = calc_tri_normal(vtx);
					}
					func(*this, vtx, nrm, i, j);
				}
			}
			triCount += ntri;
		}
	}
	return triCount;
}

void GWCollisionResource::write_geo(std::ostream& os) {
	using namespace std;

	GWVectorF* pPnts = get_pnts_top();
	Poly* pPols = get_pols_top();
	int32_t* pIdx = get_idx_top();

	os << "PGEOMETRY V5" << endl;
	os << "NPoints " << mNumPnt << " NPrims " << mNumPol << endl;
	os << "NPointGroups 0 NPrimGroups " << 0 << endl;
	os << "NPointAttrib 0 NVertexAttrib 0 NPrimAttrib 0 NAttrib 0" << endl;

	for (int i = 0; i < mNumPnt; ++i) {
		GWVectorF pnt = pPnts[i];
		os << pnt.x << " " << pnt.y << " " << pnt.z << " 1" << endl;
	}

	os << "Run " << mNumPol << " Poly" << endl;
	for (int i = 0; i < mNumPol; ++i) {
		int nvtx = pPols[i].mNumVtx;
		os << " " << nvtx << " <";
		for (int j = 0; j < nvtx; ++j) {
			os << " " << pIdx[pPols[i].mOffsIdx + j];
		}
		os << endl;
	}

	os << "beginExtra" << endl;
	os << "endExtra" << endl;
}

void GWCollisionResource::save_geo(const std::string& path) {
	std::ofstream os(path);
	if (os.bad()) return;
	write_geo(os);
	os.close();
}

void GWCollisionResource::write_tri_geo(std::ostream& os) {
	using namespace std;

	GWVectorF* pPnts = get_pnts_top();
	Poly* pPols = get_pols_top();
	int32_t* pIdx = get_idx_top();
	int32_t* pTris = get_tris_top();

	vector<GWColorF> polClrs;
	rand_pol_colors(polClrs, mNumPol);

	int ntris = calc_num_tris();

	os << "PGEOMETRY V5" << endl;
	os << "NPoints " << mNumPnt << " NPrims " << ntris << endl;
	os << "NPointGroups 0 NPrimGroups " << 0 << endl;
	os << "NPointAttrib 0 NVertexAttrib 0 NPrimAttrib 1 NAttrib 0" << endl;

	for (int i = 0; i < mNumPnt; ++i) {
		GWVectorF pnt = pPnts[i];
		os << pnt.x << " " << pnt.y << " " << pnt.z << " 1" << endl;
	}

	os << "PrimitiveAttrib" << endl;
	os << "Cd 3 float 1 1 1" << endl;

	os << "Run " << ntris << " Poly" << endl;
	for (int i = 0; i < mNumPol; ++i) {
		GWColorF clr = polClrs[i];
		int nvtx = pPols[i].mNumVtx;
		if (nvtx > 3) {
			for (int j = 0; j < nvtx - 2; ++j) {
				int triVtx[3];
				for (int k = 0; k < 3; ++k) {
					triVtx[k] = pTris[pPols[i].mOffsTris + j*3 + k];
				}
				os << " 3 <";
				for (int k = 0; k < 3; ++k) {
					os << " " << pIdx[pPols[i].mOffsIdx + triVtx[k]];
				}
				os << " [" << clr.r << " " << clr.g << " " << clr.b << "]";
				os << endl;
			}
		} else {
			os << " 3 <";
			for (int j = 0; j < nvtx; ++j) {
				os << " " << pIdx[pPols[i].mOffsIdx + j];
			}
			os << " [" << clr.r << " " << clr.g << " " << clr.b << "]" << endl;
			os << endl;
		}
	}

	os << "beginExtra" << endl;
	os << "endExtra" << endl;
}

void GWCollisionResource::save_tri_geo(const std::string& path) {
	std::ofstream os(path);
	if (os.bad()) return;
	write_tri_geo(os);
	os.close();
}

void GWCollisionResource::write_bvh_geo(std::ostream& os) {
	using namespace std;

	BVHNode* pNodes = get_bvh_top();
	if (!pNodes) return;
	int nnodes = mNumPol*2 - 1;
	int npnt = nnodes * 8;
	int npol = nnodes * 12;

	vector<GWColorF> polClrs;
	rand_pol_colors(polClrs, mNumPol);

	os << "PGEOMETRY V5" << endl;
	os << "NPoints " << npnt << " NPrims " << npol << endl;
	os << "NPointGroups 0 NPrimGroups " << 0 << endl;
	os << "NPointAttrib 0 NVertexAttrib 0 NPrimAttrib 1 NAttrib 0" << endl;

	for (int i = 0; i < nnodes; ++i) {
		GWVectorF vmin = pNodes[i].mBBoxMin;
		GWVectorF vmax = pNodes[i].mBBoxMax;
		os << vmin.x << " " << vmin.y << " " << vmin.z << " 1" << endl;
		os << vmin.x << " " << vmax.y << " " << vmin.z << " 1" << endl;
		os << vmax.x << " " << vmax.y << " " << vmin.z << " 1" << endl;
		os << vmax.x << " " << vmin.y << " " << vmin.z << " 1" << endl;

		os << vmin.x << " " << vmin.y << " " << vmax.z << " 1" << endl;
		os << vmin.x << " " << vmax.y << " " << vmax.z << " 1" << endl;
		os << vmax.x << " " << vmax.y << " " << vmax.z << " 1" << endl;
		os << vmax.x << " " << vmin.y << " " << vmax.z << " 1" << endl;
	}

	os << "PrimitiveAttrib" << endl;
	os << "Cd 3 float 1 1 1" << endl;

	os << "Run " << npol << " Poly" << endl;
	for (int i = 0; i < nnodes; ++i) {
		GWColorF clr(0.1f, 0.1f, 0.1f);
		int nodePolId = pNodes[i].get_poly_id();
		if (nodePolId >= 0) {
			clr = polClrs[nodePolId];
		}

		int org = i * 8;
		for (int j = 0; j < 2; ++j) {
			for (int k = 0; k < 4; ++k) {
				int v0 = org + j*4 + k;
				int v1 = v0 + (k < 3 ? 1 : -3);
				os << " 2 < " << v0 << " " << v1;
				os << " [" << clr.r << " " << clr.g << " " << clr.b << "]" << endl;
			}
		}
		for (int j = 0; j < 4; ++j) {
			os << " 2 < " << org + j <<  " " << org + j + 4;
			os << " [" << clr.r << " " << clr.g << " " << clr.b << "]" << endl;
		}
	}

	os << "beginExtra" << endl;
	os << "endExtra" << endl;
}

void GWCollisionResource::save_bvh_geo(const std::string& path) {
	std::ofstream os(path);
	if (os.bad()) return;
	write_bvh_geo(os);
	os.close();
}

GWCollisionResource* GWCollisionResource::load(const std::string& path) {
	GWCollisionResource* pCls = nullptr;
	GWResource* pRsrc = GWResource::load(path, GW_RSRC_ID("GWCls"));
	if (pRsrc) {
		pCls = reinterpret_cast<GWCollisionResource*>(pRsrc);
		GWSys::dbg_msg("+ collision resource: %s\n", pCls->get_path());
	}
	return pCls;
}


GWBundle* GWBundle::create(const std::string& name, const std::string& dataPath, GWRsrcRegistry* pRgy) {
	GWBundle* pBdl = nullptr;
	if (pRgy == nullptr) {
		GWSys::dbg_msg("Error: GWBundle::create - resource registry pointer can't be null");
		return nullptr;
	}
	const std::string bundleFolder = dataPath + "/" + name + "/";
	const std::string catFilePath = bundleFolder + name + ".gwcat";

	GWCatalog* pCat = GWCatalog::load(catFilePath);
	if (pCat != nullptr) {
		pBdl = new GWBundle();
		pBdl->set_name(name);
		MdlRscList* pMdlLst = &pBdl->mMdlLst;
		ImgList* pImgLst = &pBdl->mImgLst;
		MotList* pMotLst = &pBdl->mMotLst;
		ColList* pColLst = &pBdl->mColLst;

		uint32_t numRes = pCat->mNum;
		pBdl->mName = name;
		uint32_t numMdl = pCat->get_kind_count(GWResourceKind::MODEL);

		for (uint32_t i = 0; i < numRes; ++i) {
			GWResourceKind kind = pCat->get_kind(i);
			const char* pName = pCat->get_name(i);
			const char* pFileName = pCat->get_file_name(i);
			const std::string filePath = bundleFolder + pCat->get_file_name(i);
			switch (kind) {
				case GWResourceKind::MODEL: {
						GWModelResource* pMdlRsc = GWModelResource::load(filePath);
						if (pMdlRsc != nullptr) {
							pMdlLst->add(new GWListItem<GWModelResource>(pName, pMdlRsc));
						} else {
							GWSys::dbg_msg("Error loading model file %s", filePath.c_str());
						}
					}
					break;
				case GWResourceKind::DDS: {
						GWImage* pImg = GWImage::read_dds(filePath);
						if (pImg != nullptr) {
							pImgLst->add(new GWListItem<GWImage>(pName, pImg));
						} else {
							GWSys::dbg_msg("Error loading image file %s", filePath.c_str());
						}
					}
					break;
				case GWResourceKind::TDMOT: {
						GWMotion* pMot = new GWMotion();
						if (pMot->load(filePath)) {
							pMotLst->add(new GWListItem<GWMotion>(pName, pMot));
						} else {
							delete pMot;
							GWSys::dbg_msg("Error loading TD motion file %s", filePath.c_str());
						}
					}
					break;
				case GWResourceKind::COL_DATA: {
						GWCollisionResource* pColli = GWCollisionResource::load(filePath);
						if (pColli != nullptr) {
							pColLst->add(new GWListItem<GWCollisionResource>(pName, pColli));
						} else {
							GWSys::dbg_msg("Error loading cls file %s", filePath.c_str());
						}
					}
					break;
				default:
					GWSys::dbg_msg("Error: unknown resource type");
					break;
			}
		}
		pBdl->mpCat = pCat;
		pBdl->mpRegistry = pRgy;

	} else {
		GWSys::dbg_msg("Error: Cannot load %s", catFilePath.c_str());
	}

	return pBdl;
}

void GWBundle::unbind_models() {
	for (MdlRscList::Itr itr = mMdlLst.get_itr(); !itr.end(); itr.next()) {
		GWModelResource* pRsc = itr.val();
		GWResourceUtil::unbind(pRsc);
	}
	mMdlLst.purge();
}

void GWBundle::unbind_images() {
	for (ImgList::Itr itr = mImgLst.get_itr(); !itr.end(); itr.next()) {
		GWImage* pImg = itr.val();
		GWResourceUtil::unbind(pImg);
	}
}

void GWBundle::purge_models() {
	for (MdlRscList::Itr itr = mMdlLst.get_itr(); !itr.end(); itr.next()) {
		GWModelResource* pRsc = itr.val();
		GWResource::unload(pRsc);
	}
	mMdlLst.purge();
}

void GWBundle::purge_images() {
	for (ImgList::Itr itr = mImgLst.get_itr(); !itr.end(); itr.next()) {
		GWImage* pImg = itr.val();
		if (pImg) { GWImage::free(pImg); }
	}
	mImgLst.purge();
}

void GWBundle::purge_motions() {
	for (MotList::Itr itr = mMotLst.get_itr(); !itr.end(); itr.next()) {
		GWMotion* pMot = itr.val();
		pMot->unload();
	}
	mMotLst.purge();
}

void GWBundle::purge_colli_data() {
	for (ColList::Itr itr = mColLst.get_itr(); !itr.end(); itr.next()) {
		GWCollisionResource* pRsc = itr.val();
		GWResource::unload(pRsc);
	}
	mColLst.purge();
}

void GWBundle::unbind_all() {
	unbind_models();
	unbind_images();
}

void GWBundle::destroy(GWBundle* pBdl) {
	if (pBdl) {
		//pBdl->unbind_all();
		pBdl->purge_models();
		pBdl->purge_images();
		pBdl->purge_motions();
		pBdl->purge_colli_data();
		GWResource::unload(pBdl->mpCat);
		delete pBdl;
	}
}

GWRsrcRegistry* GWRsrcRegistry::create(const std::string& appPath, const std::string& relDataDir) {
	using namespace std;
	GWRsrcRegistry* pRgy = new GWRsrcRegistry();
	if (pRgy != nullptr) {
		size_t last = appPath.find_last_of('/');
		if (last == string::npos) {
			last = appPath.find_last_of('\\');
		}
		last = (last == string::npos) ? appPath.length() : last;
		pRgy->mDataPath = appPath.substr(0, last) + "/" + relDataDir;
	}
	return pRgy;
}

void GWRsrcRegistry::destroy(GWRsrcRegistry* pRgy) {
	if (pRgy != nullptr) {
		uint32_t numBundles = pRgy->mBdlLst.get_count();
		if (numBundles > 0) {
			GWBundle** ppBundles = new GWBundle*[numBundles];
			uint32_t i = 0;
			for (BundleList::Itr itr = pRgy->mBdlLst.get_itr(); !itr.end(); itr.next(), ++i) {
				ppBundles[i] = itr.val();
			}
			for (i = 0; i < numBundles; ++i) {
				pRgy->unload_bundle(ppBundles[i]);
			}
			delete[] ppBundles;
		}
		delete pRgy;
	}
}

void GWRsrcRegistry::unload_bundle(const std::string& name) {
	GWBundle::Item* pItem =  mBdlLst.find_first(name.c_str());
	if (pItem != nullptr) {
		unload_bundle(pItem->mpVal);
	}
}

void GWRsrcRegistry::unload_bundle(GWBundle* pBdl) {
	if (pBdl != nullptr) {
		if (contains_bundle(pBdl)) {
			mBdlLst.remove(&pBdl->mItem);
			GWBundle::destroy(pBdl);
		}
	}
	else {
		GWSys::dbg_msg("GWRsrcRegistry::unload_bundle: nullptr passed");
	}
}
GWBundle* GWRsrcRegistry::load_bundle(const std::string& name) {
	GWBundle* pBdl = GWBundle::create(name, mDataPath, this);
	if (pBdl != nullptr) {
		mBdlLst.add(&pBdl->mItem);
	}
	return pBdl;
}
