/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <cfloat>

#include "GWSys.hpp"
#include "GWBase.hpp"
#include "GWVector.hpp"
#include "GWOverlap.hpp"

namespace GWOverlap {

	template<typename T> bool seg_aabb(const GWVectorBase<T>& p, const GWVectorBase<T>& q, const GWVectorBase<T>& min, const GWVectorBase<T>& max) {
		GWVectorBase<T> c = (min + max) * T(0.5f);
		GWVectorBase<T> e = max - c;
		GWVectorBase<T> m = (p + q) * T(0.5f);
		GWVectorBase<T> d = q - m;
		m = m - c;
		GWVectorBase<T> ad, am;
		GWTuple::abs(ad, d);
		GWTuple::abs(am, m);
		GWVectorBase<T> rr = e + ad;
		for (int i = 0; i < 3; ++i) {
			if (am[i] > rr[i]) { return false; }
		}
		ad += FLT_EPSILON;

		if (std::fabs(m.y * d.z - m.z * d.y) > (e.y * ad.z + e.z * ad.y)) { return false; }
		if (std::fabs(m.z * d.x - m.x * d.z) > (e.x * ad.z + e.z * ad.x)) { return false; }
		if (std::fabs(m.x * d.y - m.y * d.x) > (e.x * ad.y + e.y * ad.x)) { return false; }

		return true;
	}

	template bool seg_aabb(const GWVectorBase<float>& p, const GWVectorBase<float>& q, const GWVectorBase<float>& min, const GWVectorBase<float>& max);
	template bool seg_aabb(const GWVectorBase<double>& p, const GWVectorBase<double>& q, const GWVectorBase<double>& min, const GWVectorBase<double>& max);
} //GWOverlap