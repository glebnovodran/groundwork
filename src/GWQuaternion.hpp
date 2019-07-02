/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

template<typename T> class GWQuaternionBase;

namespace GWUnitQuaternion {
	template<typename T> inline GWVectorBase<T> log(const GWQuaternionBase<T>& q);
	template<typename T>T arc_distance(const GWQuaternionBase<T>& q, const GWQuaternionBase<T>& p);
}

namespace GWQuaternion {
	template<typename T> inline GWQuaternionBase<T> exp(const GWQuaternionBase<T>& q);
	template<typename T> inline GWQuaternionBase<T> log(const GWQuaternionBase<T>& q);
	template<typename T> inline GWQuaternionBase<T> conjugate(const GWQuaternionBase<T>& q);
}

template<typename T> class GWQuaternionBase {
protected:
	GWTuple4<T> mQ;

public:
	GWQuaternionBase() = default;
	template<typename ARG_T> GWQuaternionBase(const GWQuaternionBase<ARG_T>& q) { from_tuple(q.get_tuple()); }
	GWQuaternionBase(T x, T y, T z, T w) { GWTuple::set(mQ, x, y, z, w); }

	const GWVectorBase<T> V() const { return GWVectorBase<T>(mQ.x, mQ.y, mQ.z); }
	T S() const { return mQ.w; };
	void set_vs(const GWVectorBase<T>& v, T s = 0) {
		mQ.x = v.x;
		mQ.y = v.y;
		mQ.z = v.z;
		mQ.w = s;
	}

	T* as_tptr() { return reinterpret_cast<T*>(mQ.elems); }
	const T* as_tptr() const { return reinterpret_cast<const T*>(mQ.elems); }

	template<typename TUPLE_T> void from_tuple(const TUPLE_T& tuple) { GWTuple::copy(mQ, tuple); }
	GWTuple4<T> get_tuple() const { return mQ; }

	void set_zero() {
		GWTuple::fill(mQ, T(0));
	}
	void set_identity() {
		GWTuple::set(mQ, T(0), T(0), T(0), T(1));
	}

	static GWQuaternionBase get_zero() {
		GWQuaternionBase q;
		q.set_zero();
		return q;
	}
	static GWQuaternionBase get_identity() {
		GWQuaternionBase q;
		q.set_identity();
		return q;
	}

	void set_rx(T rads) {
		T h = rads * T(0.5f);
		GWTuple::set(mQ, std::sin(h), T(0), T(0), std::cos(h));
	}
	void set_ry(T rads) {
		T h = rads * T(0.5f);
		GWTuple::set(mQ, T(0), std::sin(h), T(0), std::cos(h));
	}
	void set_rz(T rads) {
		T h = rads * T(0.5f);
		GWTuple::set(mQ, T(0), T(0), std::sin(h), std::cos(h));
	}

	void set_radians(T rx, T ry, T rz, GWRotationOrder order = GWRotationOrder::XYZ);
	void set_degrees(T dx, T dy, T dz, GWRotationOrder order = GWRotationOrder::XYZ) {
		set_radians(GWBase::radians(dx), GWBase::radians(dy), GWBase::radians(dz), order);
	}

	GWVectorBase<T> calc_axis_x() const {
		return apply(GWVectorBase<T>(1, 0, 0));
	}
	GWVectorBase<T> calc_axis_y() const {
		return apply(GWVectorBase<T>(0, 1, 0));
	}
	GWVectorBase<T> calc_axis_z() const {
		return apply(GWVectorBase<T>(0, 0, 1));
	}

	T magnitude() const { return GWTuple::magnitude(mQ); }
	void normalize() { GWTuple::normalize(mQ); }
	void normalize(const GWQuaternionBase& q) {
		GWTuple::normalize(mQ, q.mQ);
	}

	T dot(const GWQuaternionBase& q) const {
		return GWTuple::inner(mQ, q.mQ);
	}

	void conjugate(const GWQuaternionBase& q) {
		set_vs(-q.V(), q.S());
	}
	void conjugate() { conjugate(*this); }

	void invert(const GWQuaternionBase& q) {
		conjugate(q);
		T mag = magnitude(q);
		scl(GWBase::rcp0(mag));
	}
	void invert() {
		invert(*this);
	}
	GWQuaternionBase get_inverted() const {
		GWQuaternionBase q;
		q.invert(*this);
		return q;
	}

	void exp_pure(const GWVectorBase<T>& v) {
		GWVectorBase<T> vec;
		T halfAng;
		vec.normalize(v, &halfAng);
		T s = std::sin(halfAng);
		T c = std::cos(halfAng);
		vec *= s;

		set_vs(vec, c);
	}
	void exp_pure(const GWQuaternionBase& q) { exp_pure(q.V()); }
	void exp_pure() { exp_pure(*this); }

	void exp(const GWQuaternionBase& q) {
		T expS = std::exp(q.S());
		exp_pure(q);
		GWTuple::scl(mQ, expS);
	}
	void exp() { exp(*this); }

	void log(const GWQuaternionBase& q) {
		set_vs(GWUnitQuaternion::log(q), ::log(q.magnitude()));
	}
	void log() { log(*this); }

	GWVectorBase<T> expmap_encode() const { return GWUnitQuaternion::log(*this); }

	void expmap_decode(const GWVectorBase<T>& v) {
		exp_pure(v);
	}

	void pow(const GWQuaternionBase& q, T x) {
		if (q.magnitude() > T(1.0e-6f)) {
			GWQuaternionBase lq;
			lq.log(q);
			exp(lq.scl(x)); // equivalent to exp(lq * GWQuaternionBase(0, 0, 0, x));
		} else {
			set_zero();
		}
	}
	void pow(T x) { pow(*this, x); }

	void mul(const GWQuaternionBase& q, const GWQuaternionBase& p) {
		GWVectorBase<T> vq = q.V();
		GWVectorBase<T> vp = p.V();
		T sq = q.S();
		T sp = p.S();
		T s = sq*sp - vq.dot(vp);
		GWVectorBase<T> v = sq*vp + sp*vq + GWVector::cross(vq, vp);
		set_vs(v, s);
	}
	void mul(const GWQuaternionBase& q) { mul(*this, q); }

	void add(const GWQuaternionBase& q, const GWQuaternionBase& p) { GWTuple::add(*this, q, p); }
	void add(const GWQuaternionBase& q) { GWTuple::add(*this, q); }

	void sub(const GWQuaternionBase& q, const GWQuaternionBase& p) { GWTuple::sub(*this, q, p); }
	void sub(const GWQuaternionBase& q) { GWTuple::sub(*this, q); }

	void neg(const GWQuaternionBase& q) { GWTuple::neg(mQ); }
	void neg() { neg(*this); }

	void scl(const GWQuaternionBase& q, T s) { GWTuple::scl(mQ, q.mQ, s); }
	void scl(T s) { GWTuple::scl(mQ, s); }

	void diff(const GWQuaternionBase& q, const GWQuaternionBase& p) {
		*this = q * p.get_inverted();
	}
	void diff(const GWQuaternionBase& q) { diff(*this, q); }

	GWVectorBase<T> apply(const GWVectorBase<T>& v) const {
#if 0
		GWVectorBase<T> qvec = V();
		T s = S();
		T d = qvec.dot(v);
		return (d*qvec + (s*s)*v - s*GWVector::cross(v, qvec)) * T(2) - v;
#else
		GWQuaternionBase<T> q = *this;
		GWQuaternionBase<T> qv0;
		qv0.set_vs(v);
		return ((q * qv0) * GWQuaternion::conjugate(q)).V();
#endif
	}

	T arc_distance(const GWQuaternionBase& q) const {
		GWQuaternionBase norm, qnorm;
		norm.normalize(*this);
		qnorm.normalize(q);
		return GWUnitQuaternion::arc_distance(norm, qnorm);
	}
};

namespace GWUnitQuaternion {
	template<typename T> inline GWQuaternionBase<T> invert(const GWQuaternionBase<T>& q) {
		GWQuaternionBase<T> p;
		p.conjugate(q);
		return p;
		
	}
	template<typename T> inline GWVectorBase<T> log(const GWQuaternionBase<T>& q) {
		GWVectorBase<T> v = q.V();
		T vmag;
		v.normalize(&vmag);
		T h = vmag < T(0.1e-6) ? 1 : std::atan2(vmag, q.S());
		v.scl(h);
		return v;
	}

	template<typename T> GWVectorBase<T> get_radians(const GWQuaternionBase<T>& q, GWRotationOrder order = GWRotationOrder::XYZ);
	template<typename T> GWVectorBase<T> get_degrees(const GWQuaternionBase<T>& q, GWRotationOrder order = GWRotationOrder::XYZ) {
		GWVectorBase<T> r = get_radians(q, order);
		r.scl(T(180 / GWBase::pi));
		return r;
	}

	// Geodesic distance on the unit sphere
	// https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4238811/
	template<typename T> T arc_distance(const GWQuaternionBase<T>& q, const GWQuaternionBase<T>& p) {
		return T((std::acos(GWBase::saturate(std::fabs(q.dot(p)))) / (GWBase::pi / 2)));
	}

	template<typename T> GWQuaternionBase<T> slerp(const GWQuaternionBase<T>& qa, const GWQuaternionBase<T>& qb, T t);

	template<typename T> GWQuaternionBase<T> diff(const GWQuaternionBase<T>& q, const GWQuaternionBase<T>& p) {
		return q * GWUnitQuaternion::invert(p);
	}

	template<typename T> GWQuaternionBase<T> from_transform(const T* pXfrom, const int n, const bool rowAxis = true);

	// http://www.geometrictools.com/Documentation/ConstrainedQuaternions.pdf
	template<typename T> GWQuaternionBase<T> closest_by_axis(const GWQuaternionBase<T>& q, int axis) {
		GWQuaternionBase<T> res;
		res.set_identity();
		T e = q.mQ[axis];
		T w = q.mQ.w;
		T sqm = e*e + w*w;
		if (sqm > T(0)) {
			s = GWBase::rcp0(GWBase::tsqrt(sqm));
			res.mQ[axis] = s * e;
			res.w = s * w;
		}
		return res;
	}
}

namespace GWQuaternion {

	template<typename T> inline GWQuaternionBase<T> exp(const GWQuaternionBase<T>& q) {
		GWQuaternionBase<T> res;
		res.exp(q);
		return res;
	}

	template<typename T> inline GWQuaternionBase<T> log(const GWQuaternionBase<T>& q) {
		GWQuaternionBase<T> res;
		res.log(q);
		return res;
	}

	template<typename T> inline GWQuaternionBase<T> conjugate(const GWQuaternionBase<T>& q) {
		GWQuaternionBase<T> res;
		res.conjugate(q);
		return res;
	}

	template<typename T> inline GWVectorBase<T> expmap_encode(const GWQuaternionBase<T>& q) {
		return q.expmap_encode();
	}

	template<typename T> inline GWQuaternionBase<T> expmap_decode(const GWVectorBase<T>& v) {
		GWQuaternionBase<T> q;
		q.expmap_decode(v);
		return q;
	}

	template<typename T> inline T arc_distance(const GWQuaternionBase<T>& q, const GWQuaternionBase<T>& p) {
		return q.arc_distance(p);
	}

	template<typename T> GWQuaternionBase<T> diff(const GWQuaternionBase<T>& q, const GWQuaternionBase<T>& p) {
		GWQuaternionBase<T> diffQ;
		diffQ.diff(p, q);
		return diffQ;
	}
}

template<typename T> inline GWQuaternionBase<T> operator * (const GWQuaternionBase<T>& q, const GWQuaternionBase<T>& p) {
	GWQuaternionBase<T> res;
	res.mul(q, p);
	return res;
}

template<typename T> inline GWQuaternionBase<T> operator + (const GWQuaternionBase<T>& q, const GWQuaternionBase<T>& p) {
	GWQuaternionBase<T> q0 = q;
	q0.add(p);
	return q0;
}

template<typename T> inline GWQuaternionBase<T> operator - (const GWQuaternionBase<T>& q) {
	GWQuaternionBase<T> p = q;
	p.neg();
	return p;
}

template<typename T> inline GWQuaternionBase<T> operator - (const GWQuaternionBase<T>& q, const GWQuaternionBase<T>& p) {
	GWQuaternionBase<T> q0 = q;
	q0.sub(p);
	return q0;
}

typedef GWQuaternionBase<float> GWQuaternionF;
typedef GWQuaternionBase<double> GWQuaternionD;

