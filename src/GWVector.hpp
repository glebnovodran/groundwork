/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

template<typename T> class GWVectorBase :public GWTuple3<T> {
public:
	//GWVectorBase() = default;
	GWVectorBase() {
		GWTuple3<T>::x = 0;
	}
	template<typename FILL_T> GWVectorBase(FILL_T s) { GWTuple::fill(*this, s); }
	template<typename X_T, typename Y_T, typename Z_T> GWVectorBase(X_T x, Y_T y, Z_T z) { GWTuple::set(*this, x, y, z); }

	void set(const GWVectorBase& v) { GWTuple::set(*this, v); }
	void add(const GWVectorBase& v) { GWTuple::add(*this, v); }
	void sub(const GWVectorBase& v) { GWTuple::sub(*this, v); }
	void mul(const GWVectorBase& v) { GWTuple::mul(*this, v); }
	void div(const GWVectorBase& v) { GWTuple::div(*this, v); }
	template<typename SCALE_T> void scl(const SCALE_T s) { GWTuple::scl(*this, s); }

	T dot(const GWVectorBase& v) const { return GWTuple::inner(*this, v); };

	T min_elem() const { return GWTuple::min_elem(*this); }
	T max_elem() const { return GWTuple::max_elem(*this); }

	T min_abs_elem() const { return GWTuple::min_abs_elem(*this); }
	T max_abs_elem() const { return GWTuple::max_abs_elem(*this); }

	T length2() const { return dot(*this); }
	T length_fast() const { return GWTuple::magnitude_fast(*this); }
	T length() const { return GWTuple::magnitude(*this); }

	static GWVectorBase cross(const GWVectorBase& v0, const GWVectorBase& v1) {
		T x = v0.y*v1.z - v0.z*v1.y;
		T y = v0.z*v1.x - v0.x*v1.z;
		T z = v0.x*v1.y - v0.y*v1.x;
		return GWVectorBase(x, y, z);
	}

};

typedef GWVectorBase<float> GWVectorF;
typedef GWVectorBase<double> GWVectorD;
