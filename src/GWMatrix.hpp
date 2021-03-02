/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */
#include <cmath>

namespace GWMatrix {


	/* matrices */

	// Based on https://blogs.msdn.microsoft.com/nativeconcurrency/2014/09/04/raking-through-the-parallelism-tool-shed-the-curious-case-of-matrix-matrix-multiplication/
	// MxP = MxN * NxP
	template<typename DST_T, typename SRC1_T, typename SRC2_T>
	inline void mul_mm(DST_T* pDst, const SRC1_T* pSrc1, const SRC2_T* pSrc2, const int m, const int n, const int p) {
		const int nres = m * p;
		for (int i = 0; i < nres; ++i) {
			pDst[i] = 0;
		}
		for (int i = 0; i < m; ++i) {
			int ra = i * n;
			int rr = i * p;
			for (int j = 0; j < n; ++j) {
				int rb = j * p;
				DST_T s = DST_T(pSrc1[ra + j]);
				for (int k = 0; k < p; ++k) {
					pDst[rr + k] += DST_T(pSrc2[rb + k]) * s;
				}
			}
		}
	}

	// 1xN = 1xM * MxN
	template<typename DST_T, typename SRC1_T, typename SRC2_T>
	inline void mul_vm(DST_T* pDst, const SRC1_T* pVec, const SRC2_T* pMtx, const int m, const int n) {
		mul_mm(pDst, pVec, pMtx, 1, m, n);
	}

	// Mx1 = MxN * Nx1
	template<typename DST_T, typename SRC1_T, typename SRC2_T>
	inline void mul_mv(DST_T* pDst, const SRC1_T* pVec, const SRC2_T* pMtx, const int m, const int n) {
		mul_mm(pDst, pMtx, pVec, m, n, 1);
	}

	template<typename T>
	inline void scl(T* pMtx, const int n, const T s) {
		for (int i = 0; i < n*n; ++i) {
			pMtx[i] *= s;
		}
	}

	template<typename T>
	inline void add(T* pDst, int n, const T* pSrc) {
		for (int i = 0; i < n*n; ++i) {
			pDst[i] += pSrc[i];
		}
	}

	template<typename T>
	inline void set_identity(T* pMtx, const int n) {
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				int offs = i*n + j;
				pMtx[offs] = T((i == j) ? 1 : 0);
			}
		}
	}

	template<typename DST_T, typename SRC_T>
	inline void transpose(DST_T* pDst, const SRC_T* pSrc, const int n) {
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				int ij = i * n + j;
				int ji = j * n + i;
				pDst[ij] = pSrc[ji];
			}
		}
	}

	template<typename DST_T>
	inline void transpose(DST_T* pMtx, const int n) {
		for (int i = 0; i < n - 1; ++i) {
			for (int j = i + 1; j < n; ++j) {
				int ij = i * n + j;
				int ji = j * n + i;
				DST_T t = pMtx[ij];
				pMtx[ij] = pMtx[ji];
				pMtx[ji] = t;
			}
		}
	}

	template<typename T>
	inline T mtx_trace(const T* pMtx, const int n) {
		T* p = pMtx;
		T t = T(0);
		for (int i = 0; i < n; ++i) {
			t += *p;
			p += n + 1;
		}
		return t;
	}

	template<typename T>
	inline void copy(T* pDst, const T* pSrc, const int n) {
		for (int i = 0; i < n*n; ++i) {
			pDst[i] = pSrc[i];
		}
	}

	template<typename T>
	inline void copy(T* pDst, const T* pSrc, const int n, const int m) {
		for (int i = 0; i < n * m; ++i) {
			pDst[i] = pSrc[i];
		}
	}

	/* rows */

	template<typename T>
	inline void row_scl(T* pMtx, const int ncol, const int irow, const int iorg, const int iend, const T s) {
		T* p = pMtx + (irow * ncol);
		for (int i = iorg; i <= iend; ++i) {
			p[i] *= s;
		}
	}

	template<typename T>
	inline void row_elim(T* pMtx, const int ncol, const int irow1, const int irow2, const int iorg, const int iend, const T s) {
		T* p1 = pMtx + (irow1 * ncol);
		T* p2 = pMtx + (irow2 * ncol);
		for (int i = iorg; i <= iend; ++i) {
			p1[i] += p2[i] * s;
		}
	}

	template<typename T>
	inline T row_inf_norm(const T* pMtx, const int ncol, const int irow, const int iorg, const int iend, int* pIdx = nullptr) {
		if ((iorg > iend) || (iorg < 0) ) { return  T(0); }

		T* p = pMtx + (irow * ncol) + iorg;
		T maxval = std::fabs(*p);
		int idx = iorg;
		for (int i = iorg; i <= iend; ++i) {
			T val = std::fabs(*p);
			if (val > maxval) {
				maxval = val;
				idx = i;
			}
			++p;
		}
		if (pIdx) {
			*pIdx = idx;
		}
		return maxval;
	}


	/* colums */

	template<typename T>
	inline void col_scl(T* pMtx, const int ncol, const int icol, const int iorg, const int iend, const T s) {
		T* p = pMtx + (iorg * ncol) + icol;
		for (int i = iorg; i <= iend; ++i) {
			*p *= s;
			p += ncol;
		}
	}

	template<typename T>
	inline void col_elim(T* pMtx, const int ncol, const int icol1, const int icol2, const int iorg, const int iend, const T s) {
		T* p = pMtx + (iorg * ncol);
		T* p1 = p + icol1;
		T* p2 = p + icol2;
		for (int i = iorg; i <= iend; ++i) {
			*p1 += *p2 * s;
			p1 += ncol;
			p2 += ncol;
		}
	}

	template<typename T>
	inline T col_inf_norm(const T* pMtx, const int ncol, const int icol, const int iorg, const int iend, int* pIdx = nullptr) {
		if ((iorg > iend) || (iorg < 0) ) { return  T(0); }

		T* p = pMtx + (iorg * ncol) + icol;
		T maxval = std::fabs(*p);
		int idx = iorg;
		for (int i = iorg; i <= iend; ++i) {
			T val = std::fabs(*p);
			if (val > maxval) {
				maxval = val;
				idx = i;
			}
			p += ncol;
		}
		if (pIdx) {
			*pIdx = idx;
		}
		return maxval;
	}


	/* tuples */

	template<typename T>
	inline void tup_zero(T* pDst, const int iorg, const int iend) {
		for (int i = iorg; i <= iend; ++i) {
			pDst[i] = T(0);
		}
	}

	template<typename T>
	inline void tup_zero(T* pDst, const int n) {
		tup_zero(pDst, 0, n - 1);
	}

	template<typename T>
	inline void tup_sqrt(T* pDst, const T* pSrc, const int iorg, const int iend) {
		for (int i = iorg; i <= iend; ++i) {
			pDst[i] = GWBase::tsqrt(pSrc[i]);
		}
	}

	template<typename T>
	inline void tup_sqrt(T* pDst, const T* pSrc, const int n) {
		tup_sqrt(pDst, pSrc, 0, n - 1);
	}

	template<typename T>
	inline void tup_sqrt(T* pDst, const int n) {
		tup_sqrt(pDst, pDst, 0, n - 1);
	}


	template<typename T>
	inline void tup_scl(T* pDst, const T* pSrc, const int iorg, const int iend, const T s) {
		for (int i = iorg; i <= iend; ++i) {
			pDst[i] = pSrc[i] * s;
		}
	}

	template<typename T>
	inline void tup_scl(T* pDst, const T* pSrc, const int n, const T s) {
		tup_scl(pDst, pSrc, 0, n - 1, s);
	}

	template<typename T>
	inline void tup_scl(T* pDst, const int n, const T s) {
		tup_scl(pDst, pDst, 0, n - 1, s);
	}


	template<typename T>
	inline void tup_rcp(T* pDst, const T* pSrc, const int iorg, const int iend) {
		for (int i = iorg; i <= iend; ++i) {
			T s = pSrc[i];
			T r = s != T(0) ? T(1) / s : T(0);
			pDst[i] = r;
		}
	}

	template<typename T>
	inline void tup_rcp(T* pDst, const T* pSrc, const int n) {
		tup_rcp(pDst, pSrc, 0, n - 1);
	}

	template<typename T>
	inline void tup_rcp(T* pDst, const int n) {
		tup_rcp(pDst, pDst, 0, n - 1);
	}


	template<typename T>
	inline void tup_add(T* pDst, const T* pSrc, const int iorg, const int iend) {
		for (int i = iorg; i <= iend; ++i) {
			pDst[i] += pSrc[i];
		}
	}

	template<typename T>
	inline void tup_add(T* pDst, const T* pSrc, const int n) {
		tup_add(pDst, pSrc, 0, n - 1);
	}

	template<typename T>
	inline void tup_add(T* pDst, const T* pSrc1, const T* pSrc2, const int iorg, const int iend) {
		for (int i = iorg; i <= iend; ++i) {
			pDst[i] = pSrc1[i] + pSrc2[i];
		}
	}

	template<typename T>
	inline void tup_add(T* pDst, const T* pSrc1, const T* pSrc2, const int n) {
		tup_add(pDst, pSrc1, pSrc2, 0, n - 1);
	}


	template<typename T>
	inline void tup_sub(T* pDst, const T* pSrc, const int iorg, const int iend) {
		for (int i = iorg; i <= iend; ++i) {
			pDst[i] -= pSrc[i];
		}
	}

	template<typename T>
	inline void tup_sub(T* pDst, const T* pSrc, const int n) {
		tup_sub(pDst, pSrc, 0, n - 1);
	}

	template<typename T, typename CALC_T = T>
	inline CALC_T tup_inner(const T* pA, const T* pB, const int n) {
		CALC_T s = CALC_T(0);
		for (int i = 0; i < n; ++i) {
			CALC_T a = pA[i];
			CALC_T b = pB[i];
			s += a * b;
		}
		return s;
	}


	template<typename T>
	inline T tup_max(const T* pTup, const int iorg, const int iend) {
		if ((iorg > iend) || (iorg < 0) ) { return  T(0); }
		T x = pTup[iorg];
		for (int i = iorg + 1; i <= iend; ++i) {
			x = std::max(x, pTup[i]);
		}
		return x;
	}

	template<typename T>
	inline T tup_max(const T* pTup, const int n) {
		return tup_max(pTup, 0, n - 1);
	}

	template<typename T>
	inline T tup_max_abs(const T* pTup, const int iorg, const int iend) {
		if ((iorg > iend) || (iorg < 0) ) { return  T(0); }
		T x = std::fabs(pTup[iorg]);
		for (int i = iorg + 1; i <= iend; ++i) {
			x = std::max(x, T(std::fabs(pTup[i])));
		}
		return x;
	}

	template<typename T>
	inline T tup_max_abs(const T* pTup, const int n) {
		return tup_max_abs(pTup, 0, n - 1);
	}

	template<typename T>
	inline void tup_normalize(T* pDst, const T* pSrc, const int n) {
		T m = tup_max_abs(pSrc, n);
		if (m > T(0)) {
			tup_scl(pDst, pSrc, n, T(1) / m);
			tup_scl(pDst, n, T(1) / T(GWBase::tsqrt(tup_inner(pDst, pDst, n))));
		} else {
			tup_zero(pDst, n);
		}
	}

	template<typename T>
	inline bool tup_almost_eq(const T* pTup0, const T* pTup1, const int n, const T eps=0.0001f) {
		for (int i = 0; i < n; ++i) {
			if (!GWBase::almost_equal(pTup0[i], pTup1[i], eps)) { return false; }
		}
		return true;
	}

	/* swap */

	template<typename T>
	inline void swap_rows(T* pMtx, const int ncol, const int irow1, const int irow2, const int iorg, const int iend) {
		T* pRow1 = pMtx + (irow1 * ncol);
		T* pRow2 = pMtx + (irow2 * ncol);
		for (int i = iorg; i <= iend; ++i) {
			T t = pRow1[i];
			pRow1[i] = pRow2[i];
			pRow2[i] = t;
		}
	}

	template<typename T>
	inline void swap_rows(T* pMtx, const int ncol, const int irow1, const int irow2) {
		swap_rows(pMtx, ncol, irow1, irow2, 0, ncol - 1);
	}

	template<typename T>
	inline void swap_cols(T* pMtx, const int ncol, const int icol1, const int icol2, const int iorg, const int iend) {
		for (int i = iorg; i <= iend; ++i) {
			T* pCol1 = pMtx + (i * ncol) + icol1;
			T* pCol2 = pMtx + (i * ncol) + icol2;
			T t = *pCol1;
			*pCol1 = *pCol2;
			*pCol2 = t;
		}
	}


	/* inner products */

	template<typename T, typename CALC_T = T>
	inline CALC_T inner_row_vec(const T* pMtx, const int ncol, const int irow, const T* pVec, const int iorg, const int iend) {
		CALC_T s = CALC_T(0);
		const T* pRow = pMtx + (irow * ncol);
		for (int i = iorg; i <= iend; ++i) {
			CALC_T a = pRow[i];
			CALC_T b = pVec[i];
			s += a * b;
		}
		return s;
	}

	template<typename T, typename CALC_T = T>
	inline CALC_T inner_row_vec(const T* pMtx, const int ncol, const int irow, const T* pVec) {
		return inner_row_vec<T, CALC_T>(pMtx, ncol, irow, pVec, 0, ncol - 1);
	}

	template<typename T, typename CALC_T = T>
	inline CALC_T inner_col_vec(const T* pMtx, const int ncol, const int icol, const T* pVec, const int iorg, const int iend) {
		CALC_T s = CALC_T(0);
		for (int i = iorg; i <= iend; ++i) {
			const T* pCol = pMtx + (i * ncol) + icol;
			CALC_T a = *pCol;
			CALC_T b = pVec[i];
			s += a * b;
		}
		return s;
	}

	template<typename T, typename CALC_T = T>
	inline CALC_T inner_col_vec(const T* pMtx, const int ncol, const int icol, const T* pVec) {
		return inner_col_vec<T, CALC_T>(pMtx, ncol, icol, pVec, 0, ncol - 1);
	}

	template<typename T, typename CALC_T = T>
	inline CALC_T inner_row_row(const T* pMtxA, const int ncolA, const int irowA, const T* pMtxB, const int ncolB, const int irowB, const int iorg, const int iend) {
		CALC_T s = CALC_T(0);
		const T* pRowA = pMtxA + (irowA * ncolA);
		const T* pRowB = pMtxB + (irowB * ncolB);
		for (int i = iorg; i <= iend; ++i) {
			CALC_T a = pRowA[i];
			CALC_T b = pRowB[i];
			s += a * b;
		}
		return s;
	}

	template<typename T, typename CALC_T = T>
	inline CALC_T inner_row_row(const T* pMtxA, const int ncol, const int irowA, const T* pMtxB, const int irowB) {
		return inner_row_row<T, CALC_T>(pMtxA, ncol, irowA, pMtxB, ncol, irowB, 0, ncol - 1);
	}

	template<typename T, typename CALC_T = T>
	inline CALC_T inner_row_row(const T* pMtx, const int ncol, const int irow1, const int irow2) {
		return inner_row_row<T, CALC_T>(pMtx, ncol, irow1, pMtx, irow2);
	}

	template<typename T, typename CALC_T = T>
	inline CALC_T inner_row_col(const T* pMtxA, const int ncolA, const int irowA, const T* pMtxB, const int ncolB, const int icolB, const int iorg, const int iend) {
		CALC_T s = CALC_T(0);
		const T* pRowA = pMtxA + (irowA * ncolA);
		const T* pColB = pMtxB + (iorg * ncolB) + icolB;
		for (int i = iorg; i <= iend; ++i) {
			CALC_T a = pRowA[i];
			CALC_T b = *pColB;
			s += a * b;
			pColB += ncolB;
		}
		return s;
	}

	template<typename T, typename CALC_T = T>
	inline CALC_T inner_col_col(const T* pMtxA, const int ncolA, const int icolA, const T* pMtxB, const int ncolB, const int icolB, int iorg, int iend) {
		CALC_T s = CALC_T(0);
		for (int i = iorg; i <= iend; ++i) {
			const T* pColA = pMtxA + (i * ncolA) + icolA;
			const T* pColB = pMtxB + (i * ncolB) + icolB;
			CALC_T a = *pColA;
			CALC_T b = *pColB;
			s += a * b;
		}
		return s;
	}


	/* solvers */

	template<typename T>
	inline bool lu_decomp(T* pLU, const T* pMtx, const int n, T* pTmpVec /*[n]*/, int* pPerm /*[n]*/, int* pDetSgn = nullptr, const T tolerance = T(0)) {
		if (pLU != pMtx) {
			GWMatrix::copy(pLU, pMtx, n);
		}
		for (int i = 0; i < n; ++i) {
			pTmpVec[i] = inner_row_row(pMtx, n, i, i);
		}
		tup_sqrt(pTmpVec, n);
		const T epsMin = std::numeric_limits<T>::epsilon();
		T eps = epsMin;
		if (tolerance > T(0)) {
			eps = tup_max(pTmpVec, n) * tolerance;
		}
		tup_rcp(pTmpVec, n);
		int dsgn = 1;
		int offs;
		for (int j = 0; j < n; ++j) {
			T s = 0;
			int idx = 0;
			for (int i = j; i < n; ++i) {
				offs = i*n + j;
				T t = pLU[offs];
				if (j > 0) {
					t -= inner_row_col(pLU, n, i, pLU, n, j, 0, j - 1);
					pLU[offs] = t;
				}
				t = std::fabs(t) * pTmpVec[i];
				if (t > s) {
					s = t;
					idx = i;
				}
			}
			s = pLU[idx*n + j];
			if (std::fabs(s) < eps) return false;
			if (pPerm) {
				pPerm[j] = idx;
			}
			pTmpVec[idx] = pTmpVec[j];
			if (s < 0) dsgn = -dsgn;
			if (idx != j) {
				dsgn = -dsgn;
				swap_rows(pLU, n, j, idx);
			}
			if (j > 0) {
				for (int i = j + 1; i < n; ++i) {
					pLU[j*n + i] -= inner_row_col(pLU, n, j, pLU, n, i, 0, j - 1);
				}
			}
			s = T(1) / s;
			for (int i = j + 1; i < n; ++i) {
				pLU[j*n + i] *= s;
			}
		}
		if (pDetSgn) {
			*pDetSgn = dsgn;
		}
		return true;
	}

	template<typename T>
	inline void lu_get_lower(T* pL, const T* pLU, const int n) {
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				int offs = i*n + j;
				pL[offs] = j > i ? 0 : pLU[offs];
			}
		}
	}

	template<typename T>
	inline void lu_get_upper(T* pU, const T* pLU, const int n) {
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				int offs = i*n + j;
				if (i == j) {
					pU[offs] = 1;
				} else if (j > i) {
					pU[offs] = pLU[offs];
				} else {
					pU[offs] = 0;
				}
			}
		}
	}

	template<typename T>
	inline void lu_get_perm(T* pP, const int n, const int* pPerm) {
		set_identity(pP, n);
		for (int i = 0; i < n; ++i) {
			int idx = pPerm[i];
			if (idx != i) {
				swap_rows(pP, n, i, idx);
			}
		}
	}

	template<typename T>
	inline void lu_solve(T* pAns, const T* pLU, const int n, const int* pPerm, const T* pRH) {
		if (pAns == nullptr) return;
		if (pRH != nullptr) {
			if (pAns != pRH) {
				for (int i = 0; i < n; ++i) {
					pAns[i] = pRH[i];
				}
			}
		}
		for (int i = 0; i < n; ++i) {
			T t = pAns[i];
			int idx = pPerm[i];
			pAns[i] = pAns[idx];
			if (i > 0) {
				pAns[i] -= inner_row_vec(pLU, n, i, pAns, 0, i - 1);
			}
			pAns[i] /= pLU[i*n + i];
			if (idx != i) {
				pAns[idx] = t;
			}
		}
		for (int i = n - 1; --i >= 0;) {
			pAns[i] -= inner_row_vec(pLU, n, i, pAns, i + 1, n - 1);
		}
	}

	template<typename T>
	inline T lu_det(const T* pLU, const int n, const int sgn) {
		T det = 1;
		for (int i = 0; i < n; ++i) {
			det *= pLU[i*n + i];
		}
		det = std::fabs(det);
		if (sgn < 0) {
			det = -det;
		}
		return det;
	}

	template<typename T, typename XT = long double>
	inline void lu_improve(/* inout */ T* pAns, const T* pMtx, const T* pLU, const int n, const int* pPerm, const T* pRH, T* pTmpVec) {
		for (int i = 0; i < n; ++i) {
			XT r = XT(-pRH[i]);
			r += inner_row_vec<T, XT>(pMtx, n, i, pAns);
			pTmpVec[i] = T(r);
		}
		lu_solve(pTmpVec, pLU, n, pPerm, pTmpVec);
		tup_sub(pAns, pTmpVec, n);
	}

	template<typename T>
	inline void lu_inv(T* pInv, const T* pLU, const int n, const int* pPerm, T* pTmpVec /* [n] */) {
		if (pInv == pLU) {
			for (int j = n; --j >= 0;) {
				for (int i = n; --i >= j + 1;) {
					pInv[i*n + (j + 1)] = pTmpVec[i];
					pTmpVec[i] = -inner_row_col(pInv, n, j, pInv, n, i, j + 1, n - 1);
				}
				T s = T(1) / pInv[j*n + j];
				for (int i = n; --i >= j + 1;) {
					pInv[j*n + i] = pTmpVec[i];
					pTmpVec[i] = -inner_row_col(pInv, n, i, pInv, n, j, j + 1, n - 1);
				}
				pTmpVec[j] = T(1) - inner_row_col(pInv, n, j, pInv, n, j, j + 1, n - 1);
				tup_scl(pTmpVec, pTmpVec, j, n - 1, s);
			}
			for (int i = 0; i < n; ++i) {
				pInv[i*n] = pTmpVec[i];
			}
			for (int i = n - 1; --i >= 0;) {
				int idx = pPerm[i];
				if (idx != i) {
					swap_cols(pInv, n, i, idx, 0, n - 1);
				}
			}
		} else {
			for (int i = 0; i < n; ++i) {
				for (int j = 0; j < n; ++j) {
					pTmpVec[j] = T(i == j ? 1 : 0);
				}
				lu_solve(&pInv[i * n], pLU, n, pPerm, pTmpVec);
			}
			transpose(pInv, n);
		}
	}

	// pGJ - resulting mtx: inverted, no columns permutations
	// pIdxc, pIdxR, pPiv - pivot bookkeeping arrays
	// pA - matrix to solve
	// n - pA dimentions
	// pAns - solution(s)
	// pRHS - righ-hand side(s)
	// m - RHS number
	// columnRHS - RHS are passed as columns or as rows
	template<typename T>
	bool gj_solve(T* pGJ, int* pIdxC, int* pIdxR, int* pPiv, const T* pA, const int n, T* pAns = nullptr, const T* pRHS = nullptr, const int m = 0, const bool columnRHS = true) {

		int irow = 0;
		int icol = 0;

		if (pGJ != pA) { GWMatrix::copy(pGJ, pA, n); }

		if ((pAns!=nullptr) && (pRHS != nullptr)) {
			if (pAns!= pRHS) { GWMatrix::copy(pAns, pRHS, n, m); }
		}

		tup_zero(pPiv, n);

		for (int i = 0; i < n; ++i) {
			T maxElem = 0;
			for (int j = 0; j < n; ++j) {
				if (pPiv[j] != 1) {
					int rj = j * n;
					for (int k = 0; k < n; ++k) {
						if (0 == pPiv[k]) {
							T elem = pGJ[rj + k];
							if (elem < 0) elem = -elem;
							if (elem >= maxElem) {
								maxElem = elem;
								irow = j;
								icol = k;
							}
						}
					}
				}
			}

			++(pPiv[icol]);
			if (irow != icol) {
				swap_rows(pGJ, n, irow, icol);
				if (pAns) {
					if (columnRHS) {
						swap_rows(pAns, m, irow, icol, 0, m - 1);
					} else {
						swap_cols(pAns, n, irow, icol, 0, m - 1);
					}
				}
			}
			pIdxR[i] = irow;
			pIdxC[i] = icol;
			int rc = icol * n;
			int pivOffs = rc + icol;
			T pivVal = pGJ[pivOffs];
			if (pivVal == T(0)) { return false; } // singular matrix
			T invPiv = T(1) / pivVal;
			pGJ[pivOffs] = T(1);
			tup_scl(&pGJ[rc], n, invPiv);
			if (pAns) {
				if (columnRHS) {
					row_scl(pAns, m, icol, 0, m - 1, invPiv);
				} else {
					col_scl(pAns, n, icol, 0, m - 1, invPiv);
				}
			}

			for (int l = 0; l < n; ++l) {
				if (l != icol) {
					int r = l * n;
					T d = pGJ[r + icol];
					pGJ[r + icol] = T(0);
					row_elim(pGJ, n, l, icol, 0, n-1, -d);
					if (pAns) {
						if (columnRHS) {
							row_elim(pAns, m, l, icol, 0, m - 1, -d);
						} else {
							col_elim(pAns, n, l, icol, 0, m - 1, -d);
						}
					}
				}
			}
		}

		return true;
	}

	template<typename T>
	void gj_inv(T* pInv, const T* pGJ, const int n, int* pIdxC, int* pIdxR) {
		int irow = 0;
		int icol = 0;
		if (pInv != pGJ) { GWMatrix::copy(pInv, pGJ, n); }
		
		for (int i = n; --i >= 0;) {
			irow = pIdxR[i];
			icol = pIdxC[i];
			if (irow != icol) {
				swap_cols(pInv, n, irow, icol, 0, n - 1);
			}
		}
		
	}

} // namespace
