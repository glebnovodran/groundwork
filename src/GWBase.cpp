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
}
