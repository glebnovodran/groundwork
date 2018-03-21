/*
* Motion multiresolution filtering
* Author: Gleb Novodran <novodran@gmail.com>
*/

#include <groundwork.hpp>
#include "filter.hpp"

uint32_t get_bit_len(uint32_t x) {
	for (uint32_t i = 32; --i > 0;) {
		if (x & (1 << i)) return i + 1;
	}
	return 1;
}

void MotionBands::init(const GWMotion* pMot) {
	mpMot = pMot;
	uint32_t numFrames = pMot->num_frames();
	mNumBands = calc_number(numFrames);
	mNumNodes = pMot->num_nodes();
	mpNodes = new NodeBands[mNumNodes];
	for (uint32_t id = 0; id < mNumNodes; ++id) {
		mpNodes[id].pRotG = new GWVectorF[mNumBands * numFrames];
		mpNodes[id].pRotL = new GWVectorF[mNumBands * numFrames];
		mpNodes[id].numFrames = numFrames;
	}

}

uint32_t MotionBands::calc_number(uint32_t numFrames) {
	uint32_t bitLen = get_bit_len(numFrames);
	if (1 << bitLen) { return bitLen; }
	return bitLen - 1;
}

void MotionBands::build() {
	copy_g0();
	for (uint32_t lvl = 1; lvl < mNumBands; ++lvl) {
		build_low_pass(lvl);
	}
	for (uint32_t lvl = 1; lvl < mNumBands - 1; ++lvl) {
		build_band_pass(lvl);
	}
}

void MotionBands::copy_g0() {
	uint32_t numFrames = mpMot->num_frames();

	for (uint32_t i = 0; i < mNumNodes; ++i) {
		NodeBands& nodeBands = mpNodes[i];
		GWVectorF* pG = nodeBands.G(0);
		for (uint32_t fno = 0; fno < nodeBands.numFrames; ++fno) {
			*pG++ = mpMot->eval(i, GWTrackKind::ROT, fno);
		}
	}
}


void MotionBands::build_low_pass(uint32_t lvl) {
	constexpr float A = 3.0f / 8.0f;
	constexpr float B = 1.0f / 4.0f;
	constexpr float C = 1.0f / 16.0f;
	static float weight[] = {C, B, A, B, C};
	uint32_t k = lvl - 1;
	for (uint32_t i = 0; i < mNumNodes; ++i) {
		NodeBands& nodeBands = mpNodes[i];
		for (int fno = 0; fno < nodeBands.numFrames; ++fno) {
			GWVectorF sum(0.0f);
			for (int j = -2; j <= 2; ++j) {
				float w = weight[j + 2];
				int wfno = (1 << k)*j + fno;
				GWVectorF v = *nodeBands.G(k, wfno);
				v.scl(w);
				sum += v;
			}
			*nodeBands.G(k + 1, fno) = sum;
		}
	}
}

void MotionBands::build_band_pass(uint32_t lvl) {
	for (uint32_t i = 0; i < mNumNodes; ++i) {
		NodeBands& nodeBands = mpNodes[i];
		for (int fno = 0; fno < nodeBands.numFrames; ++fno) {
			*nodeBands.L(lvl, fno) = *nodeBands.G(lvl, fno) - *nodeBands.G(lvl + 1, fno);
		}
	}
}

void MultiResFilter::apply(const float * pGains) {
}
