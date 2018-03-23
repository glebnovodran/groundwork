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

	int64_t random_i64() { return s_rnd.i64(); }
}
