/*
 * Groundwork matrix operations tests
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <groundwork.hpp>
#include "test.hpp"

// https://en.wikipedia.org/wiki/Pascal_matrix
template<typename T>
inline void gen_Pascal_mtx(T* pMtx, int n) {
	using namespace GWBase;

	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			int64_t x = factorial(i + j) / (factorial(i) * factorial(j));
			pMtx[i*n + j] = T(x);
		}
	}
}

static bool test_tup() {
	using namespace GWMatrix;
	static float tuple [4] = {-1, 0, 16, -25};

	if (16 != tup_max(tuple, 4)) { return false; }
	if (25 != tup_max_abs(tuple, 4)) { return false; }
	if (16 != tup_max_abs(tuple, 0, 2)) { return false; }
	if (0 != tup_max_abs(tuple, -1, 3)) { return false; }
	return true;
}

static bool test_gj3() {
	using namespace GWBase;
	static float A[] = {
		2, -1, 2,
		1, -2, 1,
		3, -1, 2
	};
	static float invA[9];
	static float b[] = {
		10, 8, 11,
		4, 8, 2,
		10, 8, 11,
		4, 8, 2
	};
	static float bc[] = {
		10, 4, 10, 4,
		8, 8, 8, 8,
		11, 2, 11, 2
	};

	static float expected[] = {
		 1, -2, 3,
		-2, -4, 2,
		 1, -2, 3,
		-2, -4, 2
	}; // A \ b

	static float expectedC[] = {
		 1, -2, 1, -2,
		-2, -4,-2, -4,
		 3,  2, 3,  2
	};
	static int idxc[3];
	static int idxr[3];
	static float ans[12];
	static float ansI[3];
	static int pivot[3];

	static const int eps = 100;
	GWMatrix::gj_solve(invA, idxc, idxr, pivot, A, 3, ans, b, 1, false);
	GWMatrix::gj_inv(invA, invA, 3, idxc, idxr);
	GWMatrix::mul_mv(ansI, b, invA, 3, 3);

	GWMatrix::gj_solve(invA, idxc, idxr, pivot, A, 3, ans, b, 4, false);
	int ansLen = sizeof(expected) / sizeof(float);
	for (int i = 0; i < ansLen; ++i) {
		int d = f32_ulp_diff(ans[i], expected[i]);
		if (d > eps) {
			return false;
		}
	}
	GWMatrix::gj_solve(invA, idxc, idxr, pivot, A, 3, b, b, 4, false);
	for (int i = 0; i < ansLen; ++i) {
		int d = f32_ulp_diff(b[i], expected[i]);
		if (d > eps) {
			return false;
		}
	}
	GWMatrix::gj_solve(A, idxc, idxr, pivot, A, 3, ans, bc, 4, true);
	for (int i = 0; i < ansLen; ++i) {
		int d = f32_ulp_diff(ans[i], expectedC[i]);
		if (d > eps) {
			return false;
		}
	}
	return true;
}

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
			pMtx[offs] = ::fabsf(float(i - j));
		}
	}
	int dsgn = 0;
	bool res = GWMatrix::lu_decomp(pLU, pMtx, N, pTmpVec, pPerm, &dsgn);
	return res;
}

static bool test_pascal() {
	using namespace GWBase;

	const int n = 4;
	float A[n * n];
	float tmp[n];
	int perm[n];
	float LU[n * n];
	float inv[n * n];
	int ds = 0;

	float k = 1.0f / 7;

	gen_Pascal_mtx(A, n);
	GWMatrix::scl(A, n, k);

	bool res = GWMatrix::lu_decomp(LU, A, n, tmp, perm, &ds);
	if (res) {
		float det = GWMatrix::lu_det(LU, n, ds);
		float dref = ::pow(k, n); // k^n
		if (!almost_equal(det, dref)) {
			res = false;
		}
	}

	if (res) {
		GWMatrix::lu_inv(inv, LU, n, perm, tmp);

		// sign pattern test
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				int sref = int(::powf(-1.0f, float(i + j)));
				int stst = inv[i*n + j] < 0 ? -1 : 1;
				if (sref != stst) {
					res = false;
					break;
				}
			}
		}
	}

	if (res) {
		for (int i = 0; i < n*n; ++i) {
			// check if inv(A) elements are (nearly) integers
			float val = inv[i];
			float ival = ::round(val);
			int d = f32_ulp_diff(val, ival);
			if (d > 50) {
				std::cout << "ulp diff = " << d << std::endl;
				res = false;
				break;
			}
		}
	}

	return res;
}

static TEST_ENTRY s_mtx_tests[] = {
	TEST_DECL(test_tup),
	TEST_DECL(test_gj3),
	TEST_DECL(test_solve3),
	TEST_DECL(test_distmtx),
	TEST_DECL(test_pascal)
};

bool test_mtx() {
	return 0 == EXEC_TESTS(s_mtx_tests);
}


static float s_m6x5_1[] = {
	1,   2,  3,  4,  5,
	11, 12, 13, 14, 15,
	21, 22, 23, 24, 25,
	31, 32, 33, 34, 35,
	41, 42, 43, 44, 45,
	51, 52, 53, 54, 55
};

static float s_m6x5_2[] = {
	201, 202, 203, 204, 205,
	211, 212, 213, 214, 215,
	221, 222, 223, 224, 225,
	231, 232, 233, 234, 235,
	241, 242, 243, 244, 245,
	251, 252, 253, 254, 255
};

static float s_v5_1[] = { -101, -102, -103, -104, -105 };

static bool test_inner_row_vec() {
	float d = GWMatrix::inner_row_vec(s_m6x5_1, 5, 1, s_v5_1, 1, 3); // dot([12, 13, 14], [-102, -103, -104])
	return (d == -4019);
}

static bool test_inner_col_vec() {
	float d = GWMatrix::inner_col_vec(s_m6x5_1, 5, 3, s_v5_1, 1, 3); // dot([14, 24, 34], [-102, -103, -104])
	return (d == -7436);
}

static bool test_inner_row_row() {
	float d = GWMatrix::inner_row_row(s_m6x5_1, 5, 1, s_m6x5_2, 5, 2, 1, 3); // dot([12, 13, 14], [222, 223, 224])
	return (d == 8699);
}

static bool test_inner_row_col() {
	float d = GWMatrix::inner_row_col(s_m6x5_1, 5, 1, s_m6x5_2, 5, 2, 1, 3); // dot([12, 13, 14], [213, 223, 233])
	return (d == 8717);
}

static bool test_inner_col_col() {
	float d = GWMatrix::inner_col_col(s_m6x5_1, 5, 1, s_m6x5_2, 5, 2, 1, 4); // dot([12, 22, 32, 42], [213, 223, 233, 243])
	return (d == 25124);
}

static TEST_ENTRY s_inner_tests[] = {
	TEST_DECL(test_inner_row_vec),
	TEST_DECL(test_inner_col_vec),
	TEST_DECL(test_inner_row_row),
	TEST_DECL(test_inner_row_col),
	TEST_DECL(test_inner_col_col)
};

bool test_inner() {
	return 0 == EXEC_TESTS(s_inner_tests);
}
