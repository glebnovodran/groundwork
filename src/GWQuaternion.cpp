/*
* Author: Gleb Novodran <novodran@gmail.com>
*/

#include "GWBase.hpp"
#include "GWVector.hpp"
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
	GWTuple::abs(tuple);
	int numAxis = 0;
	int idx = -1;
	for (int i = 0; i < 3; ++i) {
		if (tuple[i] > 1.0e-5f) {
			++numAxis;
			idx = i;
		}
	}

	if (numAxis == 1) {
		T w = GWBase::clamp(tuple.w, T(-1.0f), T(1.0f));
		T r = ::acos(w) * 2.0f;
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
	float s = ::sin(radians[i0]);
	float c = ::cos(radians[i0]);
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

