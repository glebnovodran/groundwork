/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <ostream>

class GWColorF : public GWColorTuple4f {
public:

	GWColorF() = default;
	GWColorF(float cr, float cg, float cb, float ca = 1.0f) {
		set(cr, cg, cb, ca);
	}

	void set(float cr, float cg, float cb, float ca = 1.0f) {
		r = cr;
		g = cg;
		b = cb;
		a = ca;
	}

	void set(const GWVectorF& rgb) { set(rgb.x, rgb.y, rgb.z); }

	template<typename TUPLE_T> void from_tuple(const TUPLE_T& tuple) { GWTuple::copy(*this, tuple); }

	float luma() const {
		return r*0.299f + g*0.587f + b*0.114f;
	}

	float luminance() const {
		return r*0.212671f + g*0.71516f + b*0.072169f;
	}

	float average() const {
		return (r + g + b) / 3.0f;
	}

	void to_nonlinear(float gamma = 2.2f) {
		if (gamma <= 0.0f || gamma == 1.0f) { return; }
		float igamma = 1.0f / gamma;
		for (int i = 0; i < 3; ++i) {
			if (elems[i] <= 0.0f) {
				elems[i] = 0.0f;
			} else {
				elems[i] = ::powf(elems[i], igamma);
			}
		}
	}

	void to_linear(float gamma = 2.2f) {
		if (gamma <= 0.0f || gamma == 1.0f) { return; }
		for (int i = 0; i < 3; ++i) {
			if (elems[i] > 0.0f) {
				elems[i] = ::powf(elems[i], gamma);
			}
		}
	}

	void add(const GWColorF& c) { GWTuple::add(*this, c); }
	void add(const GWColorF& c0, const GWColorF& c1) { GWTuple::add(*this, c0, c1); }
	void add_rgb(const GWColorTuple3f& v) { GWTuple::add(*this, v); }
	void add_rgb(const GWColorF& c) {
		for (int i = 0; i < 3; ++i) {
			(*this)[i] += c[i];
		}
	}

	void sub(const GWColorF& c) { GWTuple::sub(*this, c); }
	void sub(const GWColorF& c0, const GWColorF& c1) { GWTuple::sub(*this, c0, c1); }
	void sub_rgb(const GWColorTuple3f c) { GWTuple::add(*this, c); }
	void sub_rgb(const GWColorF& c) {
		for (int i = 0; i < 3; ++i) {
			(*this)[i] -= c[i];
		}
	}

	void mul(const GWColorF& c) { GWTuple::mul(*this, c); }
	void mul(const GWColorF& c0, const GWColorF& c1) { GWTuple::mul(*this, c0, c1); }
	void mul_rgb(const GWColorTuple3f c) { GWTuple::add(*this, c); }
	void mul_rgb(const GWColorF& c) {
		for (int i = 0; i < 3; ++i) {
			(*this)[i] *= c[i];
		}	}

	void scl(const GWColorF& c, float s) { GWTuple::scl(*this, c, s); }
	void scl(float s) { GWTuple::scl(*this, s); }

	void scl_rgb(const GWColorF& c, float s) {
		for (int i = 0; i < 3; ++i) {
			(*this)[i] = c[i] * s;
		}
	}
	void scl_rgb(const GWColorTuple3f c, float s) { GWTuple::scl(*this, c, s); }
	void scl_rgb(float s) {
		for (int i = 0; i < 3; ++i) {
			(*this)[i] *= s;
		}
	}

	void clip_negative() { GWTuple::clip_low(*this, 0.0f); }
	void zero() { GWTuple::fill(*this, 0.0f); }

	uint32_t encode_rgba8() const;
	void decode_rgba8(uint32_t rgba);

	GWVectorF XYZ(const GWTransformF* pRGB2XYZ = nullptr) const;
	void from_XYZ(const GWVectorF& xyz, const GWTransformF* pXYZ2RGB = nullptr);
	
	template<typename TUPLE_T> GWColorF& operator = (const TUPLE_T& tuple) {
		GWTuple::copy(*this, tuple);
		return *this;
	}
};

std::ostream& operator << (std::ostream& os, const GWColorF& color);
