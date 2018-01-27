/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

template<typename T> class GWQuaternionBase {
protected:
	GWTuple4<T> mQ;

	void set_vs(const GWVectorBase<T>& v, T s) {
		mQ.x = v.x;
		mQ.y = v.y;
		mQ.z = v.z;
		mQ.w = s;
	}
public:
	GWQuaternionBase<T>() = default;
	GWQuaternionBase(const GWQuaternionBase& q) { from_tuple(q.mQ); }
	GWQuaternionBase<T>(T x, T y, T z, T w) { GWTuple::set(mQ, x, y, z, w); }
	GWQuaternionBase<T>(const GWVectorBase<T>& v, T s) { set_vs(v, s); }

	const GWVectorBase<T> V() const { return GWVectorBase<T>(mQ.x, mQ.y, mQ.z); }
	T S() const { return mQ.w; };

	void identity() { GWTuple::set(mQ, 0, 0, 0, 1); }

	template<typename TUPLE_T> void from_tuple(const TUPLE_T& tuple) { GWTuple::copy(mQ, tuple); }
	GWTuple4<T> get_tuple() const { return mQ; }

	void set_rx(T rads) {
		T h = rads / T(2);
		GWTuple::set(mQ, ::sin(h), T(0), T(0), ::cos(h));
	}

	void set_ry(T rads) {
		T h = rads / T(2);
		GWTuple::set(mQ, T(0), ::sin(h), T(0), ::cos(h));
	}

	void set_rz(T rads) {
		T h = rads / T(2);
		GWTuple::set(mQ, T(0), T(0), ::sin(h), ::cos(h));
	}

	void set_radians(T rx, T ry, T rz, GWRotationOrder order = GWRotationOrder::XYZ);
	void set_degrees(T dx, T dy, T dz, GWRotationOrder order = GWRotationOrder::XYZ) {
		set_radians(GWBase::radians(dx), GWBase::radians(dy), GWBase::radians(dz), order);
	}

	T magnitude() const { return GWTuple::magnitude(mQ); }
	void normalize() { return GWTuple::normalize(mQ); }
		void normalize(const GWQuaternionBase& q) {
		GWTuple::normalize_fast(mQ, q.mQ);
	}

	void conjugate(const GWQuaternionBase& q) {
		set_vs(-q.V(), q.S());
	}
	void conjugate() { conjugate(*this); }

	void exp(const GWQuaternionBase& q);
	void exp() { exp(*this); }
	void log(const GWQuaternionBase& q);
	void log() { log(*this); }
	GWVectorBase<T> expmap_encode() const;
	void expmap_decode(const GWVectorBase<T>& v);

	void mul(GWQuaternionBase<T>& q, GWQuaternionBase<T>& p) {
		GWVectorBase<T> vq = q.V();
		GWVectorBase<T> vp = p.V();
		T sq = q.S();
		T sp = p.S();
		T s = sq*sp - vq.dot(vp);
		GWVectorBase<T> v = sq*vq + sp*vq + GWVector::cross(vq, vp);
		set_vs(v, s);
	}
	void mul(GWQuaternionBase<T>& q) { mul(*this, q); }
};

namespace GWQuaternion {
	template<typename T> inline GWQuaternionBase<T> mul(const GWQuaternionBase<T>& q, const GWQuaternionBase<T>& p) {
		GWQuaternionBase<T> res;
		res.mul(q, p);
		return res;
	}
}

template<typename T> inline GWQuaternionBase<T> operator * (const GWQuaternionBase<T>& q, const GWQuaternionBase<T>& p) {
	return GWQuaternion::mul(q, p);
}

typedef GWQuaternionBase<float> GWQuaternionF;
typedef GWQuaternionBase<double> GWQuaternionD;
