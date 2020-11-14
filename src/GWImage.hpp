/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <cstring>
#include <fstream>
#include <iostream>
#include <algorithm>

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

class GWImage {
protected:
	int mWidth;
	int mHeight;
	void* mpExtMem;
	GWColorF mMin;
	GWColorF mMax;
	GWColorF mPixels[1];

	GWImage() {}

public:
	int get_width() const { return mWidth; }
	int get_height() const { return mHeight; }
	GWColorF get_min() const { return mMin; }
	GWColorF get_max() const { return mMax; }
	bool is_hdr() const {
		GWColorF minClr = get_min();
		GWColorF maxClr = get_max();
		float maxVal = GWTuple::max_elem(maxClr);
		if (maxVal > 1.0f) { return true; }
		float minVal = GWTuple::min_elem(minClr);
		return (minVal < 0.0f);
	}

	GWColorF* get_pixels() { return mPixels; }
	const GWColorF* get_pixels() const { return &mPixels[0]; }

	GWColorF get_pixel(int x, int y) const { return mPixels[(y * mWidth) + x]; }
	GWColorF get_pixel(int i) const { return mPixels[i]; }

	void set_pixel(int x, int y, const GWColorF& clr) { mPixels[(y * mWidth) + x] = clr; };

	void update();

	void write_dds(std::ofstream& ofs) const;

	static GWImage* alloc(int w, int h);
	static void free(GWImage* pImg);
	static GWImage* read_dds(std::ifstream& ifs);
	static GWImage* read_dds(const std::string& path);
	static GWImage* from_dds(const DDSHead& dds);

	void alloc_binding_memory(uint32_t size);
	void release_binding_memory();
	void set_binding_memory(void* pMem) { mpExtMem = pMem; }
	bool binding_memory_allocated() const { return mpExtMem != nullptr; }
	template<typename T> T* get_binding_memory() { return reinterpret_cast<T*>(mpExtMem); }
};
