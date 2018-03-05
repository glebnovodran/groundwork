/*
* Author: Gleb Novodran <novodran@gmail.com>
*/

#include "GWBase.hpp"
#include "GWVector.hpp"
#include "GWMatrix.hpp"
#include "GWTransform.hpp"
#include "GWQuaternion.hpp"


template<typename T> GWVectorBase<T> GWUnitQuaternion::get_radians(const GWQuaternionBase<T>& q, GWRotationOrder order) {
	static struct { uint8_t idx0, idx1, idx2, positive; } rotTbl[] = {
		/* XYZ */{ 0, 1, 2, 1 },
		/* XZY */{ 0, 2, 1, 0 },
		/* YXZ */{ 1, 0, 2, 0 },
		/* YZX */{ 1, 2, 0, 1 },
		/* ZXY */{ 2, 0, 1, 1 },
		/* ZYX */{ 2, 1, 0, 0 }
	};

	GWVectorBase<T> radians(0);
	GWTuple4<T> tuple = q.get_tuple();
	int numAxis = 0;
	int idx = -1;
	for (int i = 0; i < 3; ++i) {
		if (::fabs(tuple[i]) > 1.0e-6f) {
			++numAxis;
			idx = i;
		}
	}

	if (numAxis == 0) { return radians; }

	if (numAxis == 1) {
		T w = GWBase::clamp(tuple.w, T(-1), T(1));
		T r = ::acos(w) * 2.0f;
		r = tuple[idx] < 0 ? -r : r;
		r = GWBase::mod_pi(r);
		radians[idx] = r;
		return radians;
	}

	order = (order > GWRotationOrder::MAX) ? GWRotationOrder::XYZ : order;
	uint32_t i0 = rotTbl[(uint32_t)order].idx0;
	uint32_t i1 = rotTbl[(uint32_t)order].idx1;
	uint32_t i2 = rotTbl[(uint32_t)order].idx2;

	GWVectorBase<T> m[3];
	m[0] = q.axis_x();
	m[1] = q.axis_y();
	m[2] = q.axis_z();

	GWVectorBase<T> rotM[3] = {
		{ m[i0][i0], m[i0][i1], m[i0][i2] },
		{ m[i1][i0], m[i1][i1], m[i1][i2] },
		{ m[i2][i0], m[i2][i1], m[i2][i2] }
	};

	radians[i0] = ::atan2(rotM[1][2], rotM[2][2]);
	radians[i1] = ::atan2(-rotM[0][2], ::sqrt(rotM[0][0] * rotM[0][0] + rotM[0][1] * rotM[0][1]));
	T s = ::sin(radians[i0]);
	T c = ::cos(radians[i0]);
	radians[i2] = ::atan2(s*rotM[2][0] - c * rotM[1][0], c*rotM[1][1] - s * rotM[2][1]);

	if (rotTbl[(uint32_t)order].positive == 0) {
		radians.neg();
	}

	for (int i = 0; i < 3; ++i) {
		radians[i] = GWBase::mod_pi(radians[i]);
	}

	return radians;
}

template GWVectorBase<float> GWUnitQuaternion::get_radians(const GWQuaternionBase<float>& q, GWRotationOrder order);
template GWVectorBase<double> GWUnitQuaternion::get_radians(const GWQuaternionBase<double>& q, GWRotationOrder order);

template<typename T> GWQuaternionBase<T> GWUnitQuaternion::slerp(const GWQuaternionBase<T>& qa, const GWQuaternionBase<T>& qb, T t) {
	GWQuaternionBase<T> qres;
	GWTuple4<T> res;
	T c = qa.dot(qb);
	GWTuple4<T> a = qa.get_tuple();
	GWTuple4<T> b = qb.get_tuple();
	T theta, s, invS;
	T af;
	T bf = T(1);

	if (c < 0) {
		c = -c;
		bf = -bf;
	}

	if (::fabs(c) <= (T(1) - T(1e-5f))) {
		GWBase::clamp(c, T(-1), T(1));
		theta = ::acos(c);
		s = ::sin(theta);
		invS = T(1) / s;
		af = ::sin(((1) - t)*theta) * invS;
		bf *= ::sin(t*theta) * invS;
	} else {
		af = T(1) - t;
		bf *= t;
	}

	for (int i = 0; i < 4; ++i) {
		res[i] = af * a[i] + bf * b[i];
	}
	qres.from_tuple(res);
	return qres;
}

template GWQuaternionBase<float> GWUnitQuaternion::slerp(const GWQuaternionBase<float>& qa, const GWQuaternionBase<float>& qb, float t);
template GWQuaternionBase<double> GWUnitQuaternion::slerp(const GWQuaternionBase<double>& qa, const GWQuaternionBase<double>& qb, double t);

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
		set_identity();
	} else {
		idx *= 3;
		int iq2 = tbl[idx];
		int iq1 = tbl[idx + 1];
		int iq0 = tbl[idx + 2];
		GWQuaternionBase rq[3];
		rq[0].set_rx(rx);
		rq[1].set_ry(ry);
		rq[2].set_rz(rz);
		mul(rq[iq0], rq[iq1]);
		mul(rq[iq2]);
	}
}

template void GWQuaternionBase<float>::set_radians(float rx, float ry, float rz, GWRotationOrder order);
template void GWQuaternionBase<double>::set_radians(double rx, double ry, double rz, GWRotationOrder order);

