/*
* Author: Gleb Novodran <novodran@gmail.com>
*/

#include <fstream>
#include <cstddef>
#include "groundwork.hpp"

const char* name_from_path(const char* pPath) {
	const char* pName = nullptr;
	if (pPath) {
		const char* p = pPath + ::strlen(pPath);
		for (; --p >= pPath;) {
			if (*p == '/') {
				pName = p + 1;
				break;
			}
		}
		if (!pName) pName = pPath;
	}
	return pName;
}

GWResource* GWResource::load(const std::string& path, const char* pSig) {
	char sig[0x10];
	std::cout<<pSig<<std::endl;
	std::ifstream fs(path, std::ios::binary);
	if (fs.bad()) { return nullptr; }

	fs.read(sig, 0x10);
	if (::memcmp(sig, GW_RSRC_SIG, sizeof(GW_RSRC_SIG) - 1) != 0) return nullptr;
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
	return name_from_path(get_mtl_path(idx));
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
	uint32_t skinIdx = (uint32_t)-1;
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

GWTuple4i GWModelResource::get_pnt_skin_joints(int pntIdx) {
	GWTuple4i jnt;
	GWTuple::fill(jnt, -1);
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

GWTuple4f GWModelResource::get_pnt_skin_weights(int pntIdx) {
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

int GWModelResource::get_pnt_skin_joints_count(int pntIdx) {
	int njnt = 0;
	if (has_skin() && check_pnt_idx(pntIdx)) {
		void* pData = get_skin_data();
		bool byteIdxFlg = mNumSkinNodes <= (1 << 8);
		size_t wgtSize = byteIdxFlg ? 4 + 4 : (4 * 2) + 4;
		pData = reinterpret_cast<char*>(pData) + (pntIdx * wgtSize) + (byteIdxFlg ? 4 : 4 * 2);
		uint8_t* pWgt = reinterpret_cast<uint8_t*>(pData);
		for (int j = 0; j < 4; ++j) {
			if (pWgt[j] == 0) break;
			++njnt;
		}
	}
	return njnt;
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
