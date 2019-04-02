/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

template<typename T> struct GWVectorBase :public GWTuple3<T> {
public:
	GWVectorBase() = default;
	GWVectorBase(const GWTuple3<T>& tuple) { from_tuple(tuple); }
	GWVectorBase(const GWTuple4<T>& tuple4) { from_tuple(tuple4); }
	template<typename FILL_T> GWVectorBase(FILL_T s) { GWTuple::fill(*this, s); }
	template<typename X_T, typename Y_T, typename Z_T> GWVectorBase(X_T x, Y_T y, Z_T z) { GWTuple::set(*this, x, y, z); }

	T* as_tptr() { return reinterpret_cast<T*>(this->elems); }
	const T* as_tptr() const { return reinterpret_cast<const T*>(this->elems); }

	template<typename TUPLE_T> void from_tuple(const TUPLE_T& tuple) { GWTuple::copy(*this, tuple); }
	void fill(T val) { GWTuple::fill(*this, val); }
	void set(const GWVectorBase& v) { GWTuple::copy(*this, v); }
	void add(const GWVectorBase& v) { GWTuple::add(*this, v); }
	void add(const GWVectorBase& v0, const GWVectorBase& v1) { GWTuple::add(*this, v0, v1); }
	void sub(const GWVectorBase& v) { GWTuple::sub(*this, v); }
	void sub(const GWVectorBase& v0, const GWVectorBase& v1) { GWTuple::sub(*this, v0, v1); }
	void mul(const GWVectorBase& v) { GWTuple::mul(*this, v); }
	void mul(const GWVectorBase& v0, const GWVectorBase& v1) { GWTuple::mul(*this, v0, v1); }
	void div(const GWVectorBase& v) { GWTuple::div(*this, v); }
	void div(const GWVectorBase& v0, const GWVectorBase& v1) { GWTuple::div(*this, v0, v1); }
	void scl(const GWVectorBase& v, T s) { GWTuple::scl(*this, v, s); }
	void scl(T s) { GWTuple::scl(*this, s); }
	void neg(const GWVectorBase& v) { GWTuple::neg(*this, v); }
	void neg() { GWTuple::neg(*this); }
	void abs(const GWVectorBase& v) { GWTuple::abs(*this, v); }
	void abs() { GWTuple::abs(*this); }

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
	GWVectorBase& operator *= (T s) { scl(s); return *this; }
	GWVectorBase& operator /= (T s) { scl(1.0f / s); return *this; }
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
	GWVectorBase<T> v;
	v.add(v0, v1);
	return v;
}

template<typename T> inline GWVectorBase<T> operator - (const GWVectorBase<T>& v0, const GWVectorBase<T>& v1) {
	GWVectorBase<T> v;
	v.sub(v0, v1);
	return v;
}

template<typename T> inline GWVectorBase<T> operator - (const GWVectorBase<T>& v0) {
	GWVectorBase<T> v;
	v.neg(v0);
	return v;
}

template<typename T> inline GWVectorBase<T> operator * (const GWVectorBase<T>& v0, const GWVectorBase<T>& v1) {
	GWVectorBase<T> v;
	v.mul(v0, v1);
	return v;
}

template<typename T> inline GWVectorBase<T> operator / (const GWVectorBase<T>& v0, const GWVectorBase<T>& v1) {
	GWVectorBase<T> v;
	v.div(v0, v1);
	return v;
}

template<typename T> inline GWVectorBase<T> operator * (const GWVectorBase<T>& v0, T s) {
	GWVectorBase<T> v;
	v.scl(v0,s);
	return v;
}

template<typename T> inline GWVectorBase<T> operator * (T s, const GWVectorBase<T>& v0) {
	GWVectorBase<T> v;
	v.scl(v0, s);
	return v;
}

typedef GWVectorBase<float> GWVectorF;
typedef GWVectorBase<double> GWVectorD;
