/*
 * Groundwork matrix operations tests
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <groundwork.hpp>
#include "test.hpp"

static bool test_solve3() {
	static float A[] = {
		2, -1, 2,
		1, -2, 1,
		3, -1, 2
	};
	static float b[] = {
		10, 8, 11
	};

	float tmp[3];
	int perm[3];
	float LU[3 * 3];
	GWMatrix::lu_decomp(LU, A, 3, tmp, perm);
	float ans[3];
	GWMatrix::lu_solve(ans, LU, 3, perm, b);

	static float expected[] = { 1, -2, 3 }; // A \ b
	for (int i = 0; i < 3; ++i) {
		int d = GWBase::f32_ulp_diff(ans[i], expected[i]);
		if (d > 200) return false;
	}

	float inv[3 * 3];
	for (int i = 0; i < 3; ++i) {
		float v[3];
		for (int j = 0; j < 3; ++j) {
			v[j] = float(i == j ? 1 : 0);
		}
		GWMatrix::lu_solve(&inv[i * 3], LU, 3, perm, v);
	}
	GWMatrix::transpose(inv, 3);

	float invExpected[] = { // inv(A)
		-1, 0, 1,
		0.33333f, -0.66667f, 0,
		1.66667, -0.33333, -1
	};
	for (int i = 0; i < 3*3; ++i) {
		int d = GWBase::f32_ulp_diff(inv[i], invExpected[i]);
		if (d > 200) return false;
	}

	return true;
}

static bool test_distmtx() {
	const int N = 1000;
	float* pMtx = new float[N * N];
	float* pLU = new float[N * N];
	int* pPerm = new int[N];
	float* pTmpVec = new float[N];
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < N; ++j) {
			int offs = i*N + j;
			pMtx[offs] = ::sqrt(float(i*i) + float(j*j));
		}
	}
	int dsgn = 0;
	bool res = GWMatrix::lu_decomp(pLU, pMtx, N, pTmpVec, pPerm, &dsgn);
	return res;
}

static TEST_ENTRY s_mtx_tests[] = {
	TEST_DECL(test_solve3),
	TEST_DECL(test_distmtx)
};

bool test_mtx() {
	return 0 == EXEC_TESTS(s_mtx_tests);
}