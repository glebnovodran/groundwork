/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include "GWSys.hpp"
#include "GWBase.hpp"
#include "GWVector.hpp"
#include "GWMatrix.hpp"
#include "GWQuaternion.hpp"
#include "GWTransform.hpp"

template<typename T> void GWTransform<T>::make_rotation(T rx, T ry, T rz, GWRotationOrder order) {
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
		GWTransform r[3];
		r[0].make_rx(rx);
		r[1].make_ry(ry);
		r[2].make_rz(rz);
		idx *= 3;
		int i0 = tbl[idx];
		int i1 = tbl[idx+1];
		int i2 = tbl[idx+2];
		mul(r[i0], r[i1]);
		mul(r[i2]);
	}
}

template void GWTransform<float>::make_rotation(float rx, float ry, float rz, GWRotationOrder order);
template void GWTransform<double>::make_rotation(double rx, double ry, double rz, GWRotationOrder order);

template<typename T> void GWTransform<T>::make_transform(const GWQuaternionBase<T>& rot, const GWVectorBase<T>& trn, const GWVectorBase<T>& scl, GWTransformOrder order) {
	const uint8_t SCL = 0;
	const uint8_t ROT = 1;
	const uint8_t TRN = 2;

	static struct { uint8_t i0, i1, i2; } tbl[] = {
		{ SCL, ROT, TRN },
		{ SCL, TRN, ROT },
		{ ROT, SCL, TRN },
		{ ROT, TRN, SCL },
		{ TRN, SCL, ROT },
		{ TRN, ROT, SCL }
	};

	GWTransform m[3];
	m[SCL].make_scaling(scl);
	m[TRN].make_rotation(rot);
	m[ROT].make_translation(trn);

	uint32_t ord = (uint32_t)order;
	int i0 = tbl[ord].i0;
	int i1 = tbl[ord].i1;
	int i2 = tbl[ord].i2;

	*this = m[i0];
	mul(m[i1]);
	mul(m[i2]);
}

template void GWTransform<float>::make_transform(const GWQuaternionBase<float>& rot, const GWVectorBase<float>& trn, const GWVectorBase<float>& scl, GWTransformOrder order);
template void GWTransform<double>::make_transform(const GWQuaternionBase<double>& rot, const GWVectorBase<double>& trn, const GWVectorBase<double>& scl, GWTransformOrder order);

template<typename T> void GWTransform<T>::make_projection(T fovY, T aspect, T znear, T zfar) {
	T angle = T(0.5f) * fovY;
	T s = std::sin(angle);
	T c = std::cos(angle);
	T cot = c / s;
	T sclCoeff = zfar / (zfar - znear);
	set_zero();
	m[0][0] = cot / aspect;
	m[1][1] = cot;
	m[2][2] = -sclCoeff;
	m[3][2] = -sclCoeff * znear;
	m[2][3] = T(-1);
}

template void GWTransform<float>::make_projection(float fovY, float aspect, float znear, float zfar);
template void GWTransform<double>::make_projection(double fovY, double aspect, double znear, double zfar);

/*
template<typename T> void GWTransform<T>::transpose(const GWTransform& x) {

	for (uint32_t i = 0; i < 4; ++i) {
		for (uint32_t j = i; j < 4; ++j) {
			T val = x.m[i][j]; m[i][j] = x.m[j][i]; m[j][i] = val;
		}
	}

//	GWMatrix::transpose((T*)m, (T*)x.m, 4);
}

template void GWTransform<float>::transpose(const GWTransform& x);
template void GWTransform<double>::transpose(const GWTransform& x);
*/
template<typename T> void GWTransform<T>::transpose_sr(const GWTransform & x) {
	for (uint32_t i = 0; i < 3; ++i) {
		for (uint32_t j = i; j < 3; ++j) {
			T val = x.m[i][j]; m[i][j] = x.m[j][i]; m[j][i] = val;
		}
	}
}
template void GWTransform<float>::transpose_sr(const GWTransform& x);
template void GWTransform<double>::transpose_sr(const GWTransform& x);

template<typename T> GWTransform<T> GWTransform<T>::get_inverted_fast() const {
	GWTransform inv;

	T a0 = m[0][0] * m[1][1] - m[0][1] * m[1][0];
	T a1 = m[0][0] * m[1][2] - m[0][2] * m[1][0];
	T a2 = m[0][0] * m[1][3] - m[0][3] * m[1][0];
	T a3 = m[0][1] * m[1][2] - m[0][2] * m[1][1];
	T a4 = m[0][1] * m[1][3] - m[0][3] * m[1][1];
	T a5 = m[0][2] * m[1][3] - m[0][3] * m[1][2];

	T b0 = m[2][0] * m[3][1] - m[2][1] * m[3][0];
	T b1 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
	T b2 = m[2][0] * m[3][3] - m[2][3] * m[3][0];
	T b3 = m[2][1] * m[3][2] - m[2][2] * m[3][1];
	T b4 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
	T b5 = m[2][2] * m[3][3] - m[2][3] * m[3][2];

	T det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;

	if (T(0) == det) {
		inv.set_zero();
	} else {
		T invDet = T(1) / det;

		inv.m[0][0] = m[1][1] * b5 - m[1][2] * b4 + m[1][3] * b3;
		inv.m[1][0] = -m[1][0] * b5 + m[1][2] * b2 - m[1][3] * b1;
		inv.m[2][0] = m[1][0] * b4 - m[1][1] * b2 + m[1][3] * b0;
		inv.m[3][0] = -m[1][0] * b3 + m[1][1] * b1 - m[1][2] * b0;

		inv.m[0][1] = -m[0][1] * b5 + m[0][2] * b4 - m[0][3] * b3;
		inv.m[1][1] = m[0][0] * b5 - m[0][2] * b2 + m[0][3] * b1;
		inv.m[2][1] = -m[0][0] * b4 + m[0][1] * b2 - m[0][3] * b0;
		inv.m[3][1] = m[0][0] * b3 - m[0][1] * b1 + m[0][2] * b0;

		inv.m[0][2] = m[3][1] * a5 - m[3][2] * a4 + m[3][3] * a3;
		inv.m[1][2] = -m[3][0] * a5 + m[3][2] * a2 - m[3][3] * a1;
		inv.m[2][2] = m[3][0] * a4 - m[3][1] * a2 + m[3][3] * a0;
		inv.m[3][2] = -m[3][0] * a3 + m[3][1] * a1 - m[3][2] * a0;

		inv.m[0][3] = -m[2][1] * a5 + m[2][2] * a4 - m[2][3] * a3;
		inv.m[1][3] = m[2][0] * a5 - m[2][2] * a2 + m[2][3] * a1;
		inv.m[2][3] = -m[2][0] * a4 + m[2][1] * a2 - m[2][3] * a0;
		inv.m[3][3] = m[2][0] * a3 - m[2][1] * a1 + m[2][2] * a0;

		for (uint32_t i = 0; i < 4; ++i) {
			for (uint32_t j = 0; j < 4; ++j) {
				inv.m[i][j] *= invDet;
			}
		}
	}
	return inv;
}

template GWTransform<float> GWTransform<float>::get_inverted_fast() const;
template GWTransform<double> GWTransform<double>::get_inverted_fast() const;
