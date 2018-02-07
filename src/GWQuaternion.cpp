/*
* Author: Gleb Novodran <novodran@gmail.com>
*/

#include "GWBase.hpp"
#include "GWVector.hpp"
#include "GWQuaternion.hpp"

template<typename T> const GWQuaternionBase<T> GWQuaternionBase<T>::ZERO(0, 0, 0, 0);
template<typename T> const GWQuaternionBase<T> GWQuaternionBase<T>::IDENTITY(1, 0, 0, 0);

template<typename T> void GWQuaternionBase<T>::set_radians(T rx, T ry, T rz, GWRotationOrder order) {
	static uint8_t tbl[] = {
		0, 1, 2,
		0, 2, 1,
		1, 0, 2,
		1, 2, 0,
		2, 0, 1,
		2, 1, 0
	};
	int idx = (int)order;
	if (idx >= 6) {
		identity();
	} else {
		idx *= 3;
		int iq2 = tbl[idx];
		int iq1 = tbl[idx + 1];
		int iq0 = tbl[idx + 2];
		GWQuaternionBase<T> rq[3];
		rq[0].set_rx(rx);
		rq[1].set_ry(ry);
		rq[2].set_rz(rz);
		mul(rq[iq0], rq[iq1]);
		mul(rq[iq2]);
	}
}

template void GWQuaternionBase<float>::set_radians(float rx, float ry, float rz, GWRotationOrder order);
template void GWQuaternionBase<double>::set_radians(double rx, double ry, double rz, GWRotationOrder order);

template<typename T> void GWQuaternionBase<T>::exp(const GWQuaternionBase& q) {
	GWVectorBase<T> v;
	T halfAng;
	GWVectorBase<T> v1 = q.V();
	T hang = v1.length();

	v.normalize(q.V(), &halfAng);
	T expS = ::exp(q.S());
	T s = ::sin(halfAng);
	T c = ::cos(halfAng);
	v *= (s*expS);
	v1.scl(s/hang);
	set_vs(v, c*expS);
}

// should be normalized
template<typename T> void GWQuaternionBase<T>::log(const GWQuaternionBase& q) {
	T r = q.S();
	GWVectorBase<T> v = q.V();
	T halfAngle = ::acos(r);
	if (0 == halfAngle) {
		set_vs(v, 0);
	} else {
		T l = q.magnitude();
		v.normalize();
		v.scl(halfAngle);
		set_vs(v, 0);
	}
}

template<typename T> GWVectorBase<T> GWQuaternionBase<T>::expmap_encode() const {
	GWQuaternionBase q(*this);
	q.log();
	return q.V();
}

template<typename T> void GWQuaternionBase<T>::expmap_decode(const GWVectorBase<T>& v) {
	set_vs(v, 0);
	exp();
}

template void GWQuaternionBase<float>::exp(const GWQuaternionBase<float>& q);
template void GWQuaternionBase<double>::exp(const GWQuaternionBase<double>& q);
template void GWQuaternionBase<float>::log(const GWQuaternionBase<float>& q);
template void GWQuaternionBase<double>::log(const GWQuaternionBase<double>& q);

template GWVectorBase<float> GWQuaternionBase<float>::expmap_encode() const;
template GWVectorBase<double> GWQuaternionBase<double>::expmap_encode() const;
template void GWQuaternionBase<float>::expmap_decode(const GWVectorBase<float>& v);
template void GWQuaternionBase<double>::expmap_decode(const GWVectorBase<double>& v);
