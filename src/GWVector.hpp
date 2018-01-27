/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

template<typename T> class GWVectorBase :public GWTuple3<T> {
public:
	GWVectorBase() = default;
	GWVectorBase(const GWTuple3<T>& tuple) { from_tuple(tuple); }
	template<typename FILL_T> GWVectorBase(FILL_T s) { GWTuple::fill(*this, s); }
	template<typename X_T, typename Y_T, typename Z_T> GWVectorBase(X_T x, Y_T y, Z_T z) { GWTuple::set(*this, x, y, z); }

	template<typename TUPLE_T> void from_tuple(const TUPLE_T& tuple) { GWTuple::copy(*this, tuple); }
	void set(const GWVectorBase& v) { GWTuple::copy(*this, v); }
	void add(const GWVectorBase& v) { GWTuple::add(*this, v); }
	void add(const GWVectorBase& v0, const GWVectorBase& v1) { GWTuple::add(*this, v0, v1); }
	void sub(const GWVectorBase& v) { GWTuple::sub(*this, v); }
	void sub(const GWVectorBase& v0, const GWVectorBase& v1) { GWTuple::sub(*this, v0, v1); }
	void mul(const GWVectorBase& v) { GWTuple::mul(*this, v); }
	void div(const GWVectorBase& v) { GWTuple::div(*this, v); }
	void div(const GWVectorBase& v0, const GWVectorBase& v1) { GWTuple::div(*this, v0, v1); }
	void scl(const GWVectorBase& v, T s) { GWTuple::scl(*this, v, s); }
	void scl(T s) { GWTuple::scl(*this, s); }
	void neg(const GWVectorBase& v) { GWTuple::neg(*this, v); }
	void neg() { GWTuple::neg(*this); }

	T min_elem() const { return GWTuple::min_elem(*this); }
	T max_elem() const { return GWTuple::max_elem(*this); }

	T min_abs_elem() const { return GWTuple::min_abs_elem(*this); }
	T max_abs_elem() const { return GWTuple::max_abs_elem(*this); }

	T length_sq() const { return dot(*this); }
	T length_fast() const { return GWTuple::magnitude_fast(*this); }
	T length() const { return GWTuple::magnitude(*this); }

	void normalize(T* pMag = nullptr) { GWTuple::normalize(*this, pMag); }
	void normalize(const GWVectorBase& v, T* pMag = nullptr) { GWTuple::normalize(*this, v, pMag); }

	T dot(const GWVectorBase& v) const { return GWTuple::inner(*this, v); };

	void cross(const GWVectorBase& v0, const GWVectorBase& v1) {
		T x = v0.y*v1.z - v0.z*v1.y;
		T y = v0.z*v1.x - v0.x*v1.z;
		T z = v0.x*v1.y - v0.y*v1.x;
		GWTuple::set(*this, x, y, z);
	}
	void cross(const GWVectorBase& v) { cross(*this, v); }

	GWVectorBase& operator += (const GWVectorBase& v) { add(v); return *this; }
	GWVectorBase& operator -= (const GWVectorBase& v) { sub(v); return *this; }
	GWVectorBase& operator *= (const GWVectorBase& v) { mul(v); return *this; }
	GWVectorBase& operator /= (const GWVectorBase& v) { div(v); return *this; }
	GWVectorBase& operator *= (float s) { scl(s); return *this; }
	GWVectorBase& operator /= (float s) { scl(1.0f / s); return *this; }
};

namespace GWVector {
	template<typename T> inline T dot(const GWVectorBase<T>& v0, const GWVectorBase<T>& v1) {
		return v0.dot(v1);
	}

	template<typename T> inline GWVectorBase<T> cross(const GWVectorBase<T>& v0, const GWVectorBase<T>& v1) {
		GWVectorBase<T> res;
		res.cross(v0, v1);
		return res;
	}

	template<typename T> inline GWVectorBase<T> triple(const GWVectorBase<T>& v0, const GWVectorBase<T>& v1, const GWVectorBase<T>& v2) {
		return v0.cross(v1).dot(v2);
	}
}

template<typename T> inline GWVectorBase<T> operator + (const GWVectorBase<T>& v0, const GWVectorBase<T>& v1) {
	GWVectorBase<T> v = v0;
	v.add(v1);
	return v;
}

template<typename T> inline GWVectorBase<T> operator - (const GWVectorBase<T>& v0, const GWVectorBase<T>& v1) {
	GWVectorBase<T> v = v0;
	v.sub(v1);
	return v;
}

template<typename T> inline GWVectorBase<T> operator - (const GWVectorBase<T>& v0) {
	GWVectorBase<T> v = v0;
	v.neg();
	return v;
}

template<typename T> inline GWVectorBase<T> operator * (const GWVectorBase<T>& v0, const GWVectorBase<T>& v1) {
	GWVectorBase<T> v = v0;
	v.mul(v1);
	return v;
}

template<typename T> inline GWVectorBase<T> operator / (const GWVectorBase<T>& v0, const GWVectorBase<T>& v1) {
	GWVectorBase<T> v = v0;
	v.div(v1);
	return v;
}

template<typename T> inline GWVectorBase<T> operator * (const GWVectorBase<T>& v0, T s) {
	GWVectorBase<T> v = v0;
	v.scl(s);
	return v;
}

template<typename T> inline GWVectorBase<T> operator * (T s, const GWVectorBase<T>& v0) {
	GWVectorBase<T> v = v0;
	v.scl(s);
	return v;
}

typedef GWVectorBase<float> GWVectorF;
typedef GWVectorBase<double> GWVectorD;
