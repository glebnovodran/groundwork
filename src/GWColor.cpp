/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */
#include <iostream>

#include "GWSys.hpp"
#include "GWBase.hpp"
#include "GWColor.hpp"

std::ostream& operator << (std::ostream & os, const GWColorF & color) {
	os << "(" << color.r << ", " << color.g << ", " << color.b << ", " << color.a << ")";
	return os;
}

uint32_t GWColorF::encode_rgba8() const {
	GWColorTuple4f c;
	GWTuple::saturate(c, *this);
	GWTuple::scl(c, 255.0f);
	GWBase::Cvt32 val;
	for (int i = 0; i < 4; ++i) {
		val.b[i] = (uint8_t)c[i];
	}
	return val.u;
}

void GWColorF::decode_rgba8(uint32_t rgba) {
	GWBase::Cvt32 cvt;
	cvt.u = rgba;
	for (int i = 0; i < 4; ++i) {
		elems[i] = (float)cvt.b[i];
	}
	scl(1.0f/255.0f);
}
