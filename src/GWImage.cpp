/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */
#include <cstring>
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
	union {
		char magic[4];
		uint32_t magic32;
	};
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
		uint32_t flags;
		uint32_t fourCC;
		uint32_t bitCount;
		uint32_t maskR;
		uint32_t maskG;
		uint32_t maskB;
		uint32_t maskA;
	} format;
	uint32_t caps;
	uint32_t caps2;
	uint32_t caps3;
	uint32_t caps4;
	uint32_t reserved2;

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
			ifs.read(reinterpret_cast<char*>(pImg->mPixels), header.pitchLin);
		} else {
			pImg = alloc(w, h);
			int n = npix * 4;
			uint16_t* pTmp = new uint16_t[n];
			ifs.read(reinterpret_cast<char*>(pTmp), header.pitchLin);
			GWBase::half_to_float(reinterpret_cast<float*>(&pImg->mPixels[0]), pTmp, n);
			delete[] pTmp;
		}
	}

	if (pImg) { pImg->update(); }

	return pImg;
}

GWImage* GWImage::read_dds(const std::string& path) {
	using namespace std;
	GWImage* pImg = nullptr;
	ifstream ifs(path, ios::binary);
	if (ifs.good()) {
		pImg = read_dds(ifs);
	}
	return pImg;
}

// D3DFMT_A32B32G32R32F (dds128), no mipmap
void GWImage::write_dds(std::ofstream & ofs) const {
	DDSHead header;
	std::memset(&header, 0, sizeof(DDSHead));

	header.magic32 = 0x20534444; // "DDS "
	header.size = 124;
	header.flags = 0x081007; // DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_LINEARSIZE
	header.width = mWidth;
	header.height = mHeight;
	//header.pitchLin = ;
	header.depth = 0;
	header.mipMapCount = 0;
	header.format.size = 0x20;
	header.format.flags = 0x4;
	header.format.fourCC = 0x74; // D3DFMT_A32B32G32R32F
	header.format.bitCount = 0;
	header.format.maskR = 0;
	header.format.maskG = 0;
	header.format.maskB = 0;
	header.format.maskA = 0;
	header.pitchLin = header.width * header.height * 4 * sizeof(float);
	header.caps = 0x1000;

	ofs.write(reinterpret_cast<char*>(&header), sizeof(header));
	int w = header.width;
	int h = header.height;
	int npix = w * h;
	ofs.write(reinterpret_cast<const char*>(mPixels), npix * 4 * sizeof(float));
}
