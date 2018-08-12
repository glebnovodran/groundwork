/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include "GWBase.hpp"

namespace GWBase {
	const long double pi = ::acos((long double)-1);
	Random s_rnd;

	void set_random_seed(uint64_t seed) {
		s_rnd.set_seed(seed);
	}

	uint64_t random_u64() { return s_rnd.u64(); }

	void half_to_float(float* pDst, const uint16_t* pSrc, int n) {
		uint32_t* p = reinterpret_cast<uint32_t*>(pDst);
		for (int i = 0; i < n; ++i) {
			uint16_t h = pSrc[i];
			int32_t e = (((h >> 10) & 0x1F) + 0x70) << 23;
			uint32_t m = (h & 0x3FF) << 13;
			uint32_t s = (uint32_t)(h >> 15) << 31;
			p[i] = (e | m | s);
		}
		for (int i = 0; i < n; ++i) {
			uint16_t h = pSrc[i];
			p[i] &= (h != 0) ? 0xFFFFFFFF : 0;
		}
	}

	void vec_to_octo(float vx, float vy, float vz, float& ox, float& oy) {
		float d = 1.0f / (::fabsf(vx) + ::fabsf(vy) + ::fabsf(vz));
		ox = vx * d;
		oy = vy * d;
		if (vz < 0.0f) {
			float tx = (1.0f - fabsf(oy)) * (ox < 0.0f ? -1.0f : 1.0f);
			float ty = (1.0f - fabsf(ox)) * (oy < 0.0f ? -1.0f : 1.0f);
			ox = tx;
			oy = ty;
		}
	}

	void octo_to_vec(float ox, float oy, float& vx, float& vy, float& vz) {
		float ax = ::fabsf(ox);
		float ay = ::fabsf(oy);
		GWTuple3f v;
		v.x = ox;
		v.y = oy;
		v.z = 1.0f - ax - ay;
		if (v.z < 0.0f) {
			v.x = (1.0f - ay) * (ox < 0.0f ? -1.0f : 1.0f);
			v.y = (1.0f - ax) * (oy < 0.0f ? -1.0f : 1.0f);
		}
		GWTuple::normalize(v);
		vx = v.x; vy = v.y; vz = v.z;
	}
}
