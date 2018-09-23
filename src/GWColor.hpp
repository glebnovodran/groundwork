/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

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

	void add(const GWColorF& c) { GWTuple::add(*this, c); }
	void add(const GWColorF& c0, const GWColorF& c1) { GWTuple::add(*this, c0, c1); }

	void sub(const GWColorF& c) { GWTuple::sub(*this, c); }
	void sub(const GWColorF& c0, const GWColorF& c1) { GWTuple::sub(*this, c0, c1); }

	void mul(const GWColorF& c) { GWTuple::mul(*this, c); }
	void mul(const GWColorF& c0, const GWColorF& c1) { GWTuple::mul(*this, c0, c1); }

	void scl(const GWColorF& c, float s) { GWTuple::scl(*this, c, s); }
	void scl(float s) { GWTuple::scl(*this, s); }

	void scl_rgb(const GWColorF& c, float s) {
		for (int i = 0; i < 3; ++i) {
			(*this)[i] = c[i] * s;
		}
	}
	void scl_rgb(float s) {
		for (int i = 0; i < 3; ++i) {
			(*this)[i] *= s;
		}
	}

	void clip_negative() { GWTuple::clip_low(*this, 0.0f); }
	void zero() { GWTuple::fill(*this, 0.0f); }

	uint32_t encode_rgba8() const;

	template<typename TUPLE_T> GWColorF& operator = (const TUPLE_T& tuple) {
		GWTuple::copy(*this, tuple);
		return *this;
	}
};

std::ostream& operator << (std::ostream& os, const GWColorF& color);
