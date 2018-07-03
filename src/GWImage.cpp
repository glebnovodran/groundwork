/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */
#include "GWBase.hpp"
#include "GWVector.hpp"
#include "GWColor.hpp"
#include "GWImage.hpp"

GWImage* GWImage::alloc(int w, int h) {
	int numPix = w * h;
	size_t numByte = sizeof(GWImage) + sizeof(GWColorF) * (numPix - 1);
	GWImage* pImg = reinterpret_cast<GWImage*>(new char[numByte]);
	pImg->mWidth = w;
	pImg->mHeight = h;
	for (int i = 0; i < numPix; ++i) {
		pImg->mPixels[i].zero();
	}
	return pImg;
}

void GWImage::free(GWImage * pImg) {
	if (pImg) {
		delete[] reinterpret_cast<char*>(pImg);
	}
}

static void calc_range(GWColorF& minVal, GWColorF& maxVal, const GWColorF* pPix, int n) {
	minVal = pPix[0];
	maxVal = pPix[0];
	for (int i = 1; i < n; ++i) {
		GWTuple::min(minVal, minVal, pPix[i]);
		GWTuple::max(maxVal, maxVal, pPix[i]);
	}
}

void GWImage::update() {
	calc_range(mMin, mMax, mPixels, mWidth * mHeight);
}

struct DDSHead {
	char magic[4];
	uint32_t size;
	uint32_t flags;
	uint32_t height;
	uint32_t width;
	uint32_t pitchLin;
	uint32_t depth;
	uint32_t mipMapCount;
	uint32_t reserved1[11];
	struct PixelFormat {
		uint32_t size;
		uint32_t dlags;
		uint32_t fourCC;
		uint32_t bitCount;
		uint32_t maskR;
		uint32_t maskG;
		uint32_t maskB;
		uint32_t maskA;
	} format;
	uint32_t caps1;
	uint32_t caps2;
	uint32_t reserved2[3];

	bool is_dds() const {
		static char sig[4] = { 'D', 'D', 'S', ' ' };
		return std::memcmp(sig, magic, 4) == 0;
	}
	bool is_dds128() const { return format.fourCC == 0x74; }
	bool is_dds64() const { return format.fourCC == 0x71; }
};

GWImage* GWImage::read_dds(std::ifstream& ifs) {
	DDSHead header;
	ifs.read(reinterpret_cast<char*>(&header), sizeof(header));
	GWImage* pImg = nullptr;

	if (header.is_dds()) {
		int w = header.width;
		int h = header.height;
		int npix = w * h;
		if (header.is_dds128()) {
			pImg = alloc(w, h);
			ifs.read(reinterpret_cast<char*>(pImg->mPixels), npix * sizeof(GWColorF));
		} else {
			pImg = alloc(w, h);
			int n = npix * 4;
			uint16_t* pTmp = new uint16_t[n];
			ifs.read(reinterpret_cast<char*>(pTmp), n * sizeof(uint16_t));
			GWBase::half_to_float(reinterpret_cast<float*>(&pImg->mPixels[0]), pTmp, n);
			delete[] pTmp;
		}
	}

	if (pImg) { pImg->update(); }

	return pImg;
}
