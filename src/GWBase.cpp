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
}
