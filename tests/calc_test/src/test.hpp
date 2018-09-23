/*
 * Groundwork calculation test utils
 * Author: Gleb Novodran <novodran@gmail.com>
 */
#include <cstdint>
#include <iostream>
#include <iomanip>

union U32 {
	int32_t i;
	uint32_t u;
	float f;
};

inline int f32_ulp_diff(float a, float b) {
	if (a == b) return 0;
	const float e = 1.0e-6f;
	if (::fabs(a) < e && ::fabs(b) < e) return 1;
	U32 ua;
	ua.f = a;
	U32 ub;
	ub.f = b;
	if (ua.i < 0) ua.i = (1U << 31) - ua.i;
	if (ub.i < 0) ub.i = (1U << 31) - ub.i;
	return ua.u > ub.u ? ua.u - ub.u : ub.u - ua.u;
}

double time_micros();

struct TEST_ENTRY {
	bool(*pFunc)();
	const char* pName;
};

#define TEST_DECL(_name) { _name, #_name }
#define NTESTS(_ary) ( sizeof(_ary) / sizeof((_ary)[0]) )
#define EXEC_TESTS(_ary) exec_tests(_ary, NTESTS((_ary)))

inline bool exec_tests(TEST_ENTRY* pTests, int n) {
	int nfail = 0;
	for (int i = 0; i < n; ++i) {
		if (!pTests[i].pFunc()) {
			std::cerr << "! " << pTests[i].pName << std::endl;
			++nfail;
		}
	}
	return nfail == 0;
}
