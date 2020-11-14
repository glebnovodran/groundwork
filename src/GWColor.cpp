/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */
#include <iostream>

#include "GWSys.hpp"
#include "GWBase.hpp"
#include "GWVector.hpp"
#include "GWMatrix.hpp"
#include "GWQuaternion.hpp"
#include "GWTransform.hpp"
#include "GWColor.hpp"

namespace GWColor {
	static inline const GWTransformF* get_RGB2XYZ(const GWTransformF* pRGB2XYZ) {
		// Rec. 709; 1931 CIE, D65
		static const GWTransformF s_rec709_RGB2XYZ = {
			0.412453f, 0.212671f, 0.019334f, 0.0f,
			0.357580f, 0.715160f, 0.119193f, 0.0f,
			0.180423f, 0.072169f, 0.950227f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
		return pRGB2XYZ ? pRGB2XYZ : &s_rec709_RGB2XYZ;
	}

	static inline const GWTransformF* get_XYZ2RGB(const GWTransformF* pXYZ2RGB) {
		// Rec. 709; 1931 CIE, D65
		static const GWTransformF s_rec709_XYZ2RGB = {
			3.240479f, -0.969256f,  0.055648f, 0.0f,
			-1.537150f,  1.875992f, -0.204043f, 0.0f,
			-0.498535f,  0.041556f,  1.057311f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
		return pXYZ2RGB ? pXYZ2RGB : &s_rec709_XYZ2RGB;
	}

	GWVectorF XYZ_to_xyY(const GWVectorF& xyz) {
		GWVectorF s = xyz * GWBase::rcp0(xyz.x + xyz.y + xyz.z);
		return GWVectorF(s.x, s.y, xyz.y);
	}

	GWVectorF xyY_to_XYZ(const GWVectorF& xyY) {
		float x = xyY.x;
		float y = xyY.y;
		float z = 1.0f - x - y;
		float Y = xyY.z;
		float s = GWBase::div0(Y, y);
		return GWVectorF(x * s, Y, z * s);
	}
} // namespace

GWVectorF GWColorF::XYZ(const GWTransformF* pRGB2XYZ) const {
	const GWTransformF* pXform = GWColor::get_RGB2XYZ(pRGB2XYZ);
	return pXform->calc_vec(GWVectorF(r, g, b));
}

void GWColorF::from_XYZ(const GWVectorF& xyz, const GWTransformF* pXYZ2RGB) {
	const GWTransformF* pXform = GWColor::get_XYZ2RGB(pXYZ2RGB);
	GWVectorF rgb = pXform->calc_vec(xyz);
	set(rgb);
}

GWVectorF GWColorF::xyY(const GWTransformF* pRGB2XYZ) const {
	return GWColor::XYZ_to_xyY(XYZ(pRGB2XYZ));
}

void GWColorF::from_xyY(const GWVectorF& xyY, const GWTransformF* pXYZ2RGB) {
	from_XYZ(GWColor::xyY_to_XYZ(xyY), pXYZ2RGB);
}

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

