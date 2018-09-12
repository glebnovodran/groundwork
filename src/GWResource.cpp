/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <fstream>
#include <cstddef>
#include "groundwork.hpp"

const char* GWResourceUtil::name_from_path(const char* pPath, char sep) {
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

const char* GWResourceUtil::get_kind_string(GWResourceKind kind) {
	const char* pStr = "UNKNOWN";
	switch (kind) {
	case GWResourceKind::CATALOG: pStr = "Catalogue"; break;
	case GWResourceKind::MODEL: pStr = "Model"; break;
	case GWResourceKind::DDS: pStr = "DDS"; break;
	case GWResourceKind::TDMOT: pStr = "TDMotion"; break;
	case GWResourceKind::TDGEO: pStr = "TDGeometry"; break;
	}
	return pStr;
}

void write_py_mtx(std::ostream& os, const GWTransformF& xform) {
	os << "[";
	for (int i = 0; i < 4; ++i) {
		os << "[";
		for (int j = 0; j < 4; ++j) {
			os << xform.m[i][j];
			if (j < 3) os << ", ";
		}
		os << "]";
		if (i < 3) os << ", ";
	}
	os << "]";
}

GWResource* GWResource::load(const std::string& path, const char* pSig) {
	char sig[0x10];
	std::cout<<pSig<<std::endl;
	std::ifstream fs(path, std::ios::binary);
	if (fs.bad()) { return nullptr; }

	fs.read(sig, 0x10);
	if (::memcmp(sig, GW_RSRC_SIG, sizeof(GW_RSRC_SIG) - 1) != 0) { return nullptr; }
	if (pSig) {
		if (::strcmp(sig, pSig) != 0) { return nullptr; }
	}

	fs.seekg(offsetof(GWResource, mDataSize));
	uint32_t size = 0;
	fs.read((char*)&size, 4);
	if (size < 0x10) return nullptr;
	char* pBuf = new char[size];
	if (pBuf) {
		fs.seekg(std::ios::beg);
		fs.read(pBuf, size);
	}

	fs.close();
	return reinterpret_cast<GWResource*>(pBuf);
}

const char* GWModelResource::get_mtl_name(uint32_t idx) {
	return GWResourceUtil::name_from_path(get_mtl_path(idx));
}

GWTransformF GWModelResource::calc_skel_node_world_mtx(uint32_t idx, const GWTransformF* pLM, GWTransformF* pParentWM) {
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
			parentWM = pLM[idx] * parentWM;
			idx = get_skel_node_parent_idx(idx);
		}
		nodeWM = parentWM * nodeWM;
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
			os << "# "; write_py_mtx(os, calc_skel_node_world_mtx(i)); os << endl;
			os << "nd = hou.node('" << pBase << "').createNode('null', '" << pNodeName << "')" << endl;
			GWTransformF lm = get_skel_node_local_mtx(i);
			os << "nd.setParmTransform(hou.Matrix4(";
			write_py_mtx(os, lm);
			os << "))" << endl;
			bool skinFlg = is_skel_node_skin_deformer(i);
			os << "nd.setParms({'geoscale':0.01,'controltype':1})" << endl;
			os << "nd.setUserData('nodeshape', '" << (skinFlg ? "bone" : "rect") << "')" << endl;
			if (skinFlg) {
				//glm::vec4 sph = calc_skin_node_sphere_of_influence(find_skel_node_skin_idx(i));
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

