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

