/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

template<typename T> class GWVectorBase :public GWTuple3<T> {
public:
	GWVectorBase() = default;
	template<typename FILL_T> GWVectorBase(FILL_T s) { GWTuple::fill(*this, s); }
	template<typename X_T, typename Y_T, typename Z_T> GWVectorBase(X_T x, Y_T y, Z_T z) { GWTuple::set(*this, x, y, z); }

	void set(const GWVectorBase& v) { GWTuple::set(*this, v); }
	void add(const GWVectorBase& v) { GWTuple::add(*this, v); }
	void sub(const GWVectorBase& v) { GWTuple::sub(*this, v); }
	void mul(const GWVectorBase& v) { GWTuple::mul(*this, v); }
	void div(const GWVectorBase& v) { GWTuple::div(*this, v); }
	template<typename SCALE_T> void scl(const SCALE_T s) { GWTuple::scl(*this, s); }

	typename GWTuple3<T>::elem_t dot(const GWVectorBase& v) { return GWTuple::inner(*this, v); };


	T max_abs_elem() const;
	T length2() const { dot(*this); }
	T length_fast() const { ::sqrtf(length2); }
	T length() const;

	static GWVectorBase cross(const GWVectorBase& v0, const GWVectorBase& v1) {
		T x = v0.y*v1.z - v0.z*v1.y;
		T y = v0.z*v1.x - v0.x*v1.z;
		T z = v0.x*v1.y - v0.y*v1.x;
		return GWVectorBase(x, y, z);
	}
};

typedef GWVectorBase<float> GWVectorF;
typedef GWVectorBase<double> GWVectorD;
