/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include "GWBase.hpp"
#include "GWVector.hpp"
#include "GWMatrix.hpp"
#include "GWTransform.hpp"
#include "GWQuaternion.hpp"

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
	m[TRN] = rot.get_transform();
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
