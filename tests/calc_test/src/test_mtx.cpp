/*
 * Groundwork matrix operations tests
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <groundwork.hpp>
#include "test.hpp"

static bool test_solve3() {
	using namespace GWBase;

	static float A[] = {
		2, -1, 2,
		1, -2, 1,
		3, -1, 2
	};
	static float b[] = {
		10, 8, 11
	};
	/*
		A = [2, -1, 2; 1, -2, 1; 3, -1, 2]
		b = [10, 8, 11]'
		A \ b
	 */

	float tmp[3];
	int perm[3];
	float LU[3 * 3];
	float L[3 * 3];
	float U[3 * 3];
	float P[3 * 3];
	float Q[3 * 3];
	float PtL[3 * 3];
	int ds = 0;

	GWMatrix::lu_decomp(LU, A, 3, tmp, perm, &ds);
	GWMatrix::lu_get_lower(L, LU, 3);
	GWMatrix::lu_get_upper(U, LU, 3);
	GWMatrix::lu_get_perm(P, 3, perm);
	// A == P'*L*U
	GWMatrix::transpose(P, 3);
	GWMatrix::mul_mm(PtL, P, L, 3, 3, 3);
	GWMatrix::mul_mm(Q, PtL, U, 3, 3, 3);
	for (int i = 0; i < 3*3; ++i) {
		int d = f32_ulp_diff(Q[i], A[i]);
		if (d > 200) return false;
	}

	float ans[3];
	GWMatrix::lu_solve(ans, LU, 3, perm, b);
	GWMatrix::lu_improve(ans, A, LU, 3, perm, b, tmp);
	float det = GWMatrix::lu_det(LU, 3, ds); // det(A)
	if (f32_ulp_diff(det, 3.0f) > 100) {
		return false;
	}

	static float expected[] = { 1, -2, 3 }; // A \ b
	int dmax = 0;
	for (int i = 0; i < 3; ++i) {
		int d = f32_ulp_diff(ans[i], expected[i]);
		dmax = std::max(d, dmax);
		if (d > 200) return false;
	}

	float inv[3 * 3];
	float v[3];
#if 1
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			v[j] = float(i == j ? 1 : 0);
		}
		GWMatrix::lu_solve(&inv[i * 3], LU, 3, perm, v);
		GWMatrix::lu_improve(&inv[i * 3], A, LU, 3, perm, v, tmp);
	}
	GWMatrix::transpose(inv, 3);
#else
	GWMatrix::lu_inv(inv, LU, 3, perm, v);
#endif

	float invExpected[] = { // inv(A)
		-1, 0, 1,
		0.33333f, -0.66667f, 0,
		1.66667, -0.33333, -1
	};
	int dmaxInv = 0;
	for (int i = 0; i < 3*3; ++i) {
		int d = f32_ulp_diff(inv[i], invExpected[i]);
		dmaxInv = std::max(d, dmaxInv);
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