/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <fstream>
#include <iostream>
#include <algorithm>

class GWImage {
protected:
	int mWidth;
	int mHeight;
	int mReserved0;
	int mReserved1;
	GWColorF mMin;
	GWColorF mMax;
	GWColorF mPixels[1];

	GWImage() {}

public:
	int get_width() const { return mWidth; }
	int get_height() const { return mHeight; }
	GWColorF get_min() const { return mMin; }
	GWColorF get_max() const { return mMax; }
	GWColorF* get_pixels() { return mPixels; }
	GWColorF get_pixel(int x, int y) const { return mPixels[(y * mWidth) + x]; }
	void set_pixel(int x, int y, const GWColorF& clr) { mPixels[(y * mWidth) + x] = clr; };

	void update();

	void write_dds(std::ofstream& ofs) const;

	static GWImage* alloc(int w, int h);
	static void free(GWImage* pImg);
	static GWImage* read_dds(std::ifstream& ifs);
	static GWImage* read_dds(const std::string& path);

};
