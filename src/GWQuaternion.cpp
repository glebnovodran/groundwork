/*
* Author: Gleb Novodran <novodran@gmail.com>
*/

#include "GWBase.hpp"
#include "GWVector.hpp"
#include "GWQuaternion.hpp"

namespace GWQuaternion {
	template<typename T> inline void set_radians(GWQuaternionBase<T>& q, T rx, T ry, T rz, GWRotationOrder order) {
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
			q.identity();
		} else {
			idx *= 3;
			int iq2 = tbl[idx];
			int iq1 = tbl[idx + 1];
			int iq0 = tbl[idx + 2];
			GWQuaternionBase<T> rq[3];
			rq[0].set_rx(rx);
			rq[1].set_ry(ry);
			rq[2].set_rz(rz);
			q.mul(rq[iq0], rq[iq1]);
			q.mul(rq[iq2]);
		}
	}
}

template<> void GWQuaternionBase<float>::set_radians(float rx, float ry, float rz, GWRotationOrder order) {
	GWQuaternion::set_radians(*this, rx, ry, rz, order);
}

template<> void GWQuaternionBase<double>::set_radians(double rx, double ry, double rz, GWRotationOrder order) {
	GWQuaternion::set_radians(*this, rx, ry, rz, order);
}

