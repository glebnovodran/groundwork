/*
 * Groundwork calculation test utils
 * Author: Gleb Novodran <novodran@gmail.com>
 */
#include <cstdint>
#include <iostream>
#include <iomanip>

#define COMPARE_QUAT(q, p) GWBase::almost_equal(q.arc_distance(p), 0.0f)
#define COMPARE_VEC(v0, v1, eps) GWTuple::compare(v0, v1, eps)

template<typename T>
bool compare_mtx(const T* mtx0, const T* mtx1, int m, int n) {
	for (int i = 0; i < m; ++i) {
		int ri = i * n;
		if (!GWMatrix::tup_almost_eq(&mtx0[ri], &mtx1[ri], n, 0.001f)) { return false; }
	}
	return true;
}

struct TEST_ENTRY {
	bool(*pFunc)();
	const char* pName;
};

#define TEST_DECL(_name) { _name, #_name }
#define NTESTS(_ary) ( sizeof(_ary) / sizeof((_ary)[0]) )
#define EXEC_TESTS(_ary) exec_tests(_ary, NTESTS((_ary)))

inline int exec_tests(TEST_ENTRY* pTests, int n) {
	int nfail = 0;
	for (int i = 0; i < n; ++i) {
		if (!pTests[i].pFunc()) {
			std::cerr << std::endl << "! " << pTests[i].pName;
			++nfail;
		}
	}
	return nfail;
}

bool test_quat();
bool test_mtx();
bool test_xform();
bool test_inner();

int run_all_tests();
