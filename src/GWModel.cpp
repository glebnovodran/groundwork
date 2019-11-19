/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include "groundwork.hpp"

GWModel* GWModel::create(GWModelResource* pMdr, const size_t paramMemSz, const size_t extMemSz) {
	if (pMdr == nullptr) { return nullptr; }

	GWModel* pMdl = nullptr;
	size_t memSz = GWBase::align(sizeof(GWModel), 0x10);
	size_t offsSkinXforms = 0;
	size_t offsSkelXforms = 0;
	size_t offsParamMem = 0;
	size_t offsExtMem = 0;

	uint32_t numSkin = pMdr->mNumSkinNodes;
	uint32_t numSkel = pMdr->mNumSkelNodes;


	if (pMdr->has_skin()) {
		offsSkinXforms = memSz;
		memSz += numSkin * sizeof(GWTransform3x4F);
	}

	if (pMdr->has_skel()) {
		offsSkelXforms = memSz;
		memSz += 2 * numSkel * sizeof(GWTransform3x4F);
	}

	if (paramMemSz) {
		offsParamMem = memSz;
		memSz += paramMemSz;
	}
	if (extMemSz) {
		offsExtMem = memSz;
		memSz += extMemSz;
	}
	//
	uint8_t* pMem = new uint8_t[memSz];
	std::fill_n(pMem, memSz, 0);

	pMdl = reinterpret_cast<GWModel*>(pMem);
	pMdl->mpRsc = pMdr;

	if (pMdr->has_skin()) {
		pMdl->mpSkinXforms = reinterpret_cast<GWTransform3x4F*>(GWBase::incr_ptr(pMem, offsSkinXforms));
	}

	if (pMdr->has_skel()) {
		pMdl->mpSkelXforms = reinterpret_cast<GWTransform3x4F*>(GWBase::incr_ptr(pMem, offsSkelXforms));
	}

	if (paramMemSz) {
		pMdl->mpParamMem = GWBase::incr_ptr(pMem, offsParamMem);
	}
	if (extMemSz) {
		pMdl->mpExtMem = GWBase::incr_ptr(pMem, extMemSz);
	}
	pMdl->mWorld.set_identity();
	return pMdl;
}
