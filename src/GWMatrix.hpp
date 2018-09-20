/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

namespace GWMatrix {
	// Based on https://blogs.msdn.microsoft.com/nativeconcurrency/2014/09/04/raking-through-the-parallelism-tool-shed-the-curious-case-of-matrix-matrix-multiplication/
	// MxP = MxN * NxP
	template<typename DST_T, typename SRC1_T, typename SRC2_T>
	inline void mul_mm(DST_T* pDst, const SRC1_T* pSrc1, const SRC2_T* pSrc2, int M, int N, int P) {
		const int nres = M * P;
		for (int i = 0; i < nres; ++i) {
			pDst[i] = 0;
		}
		for (int i = 0; i < M; ++i) {
			int ra = i * N;
			int rr = i * P;
			for (int j = 0; j < N; ++j) {
				int rb = j * P;
				DST_T s = DST_T(pSrc1[ra + j]);
				for (int k = 0; k < P; ++k) {
					pDst[rr + k] += DST_T(pSrc2[rb + k]) * s;
				}
			}
		}
	}

	// 1xN = 1xM * MxN
	template<typename DST_T, typename SRC1_T, typename SRC2_T>
	inline void mul_vm(DST_T* pDst, const SRC1_T* pVec, const SRC2_T* pMtx, int M, int N) {
		mul_mm(pDst, pVec, pMtx, 1, M, N);
	}

	// Mx1 = MxN * Nx1
	template<typename DST_T, typename SRC1_T, typename SRC2_T>
	inline void mul_mv(DST_T* pDst, const SRC1_T* pVec, const SRC2_T* pMtx, int M, int N) {
		mul_mm(pDst, pMtx, pVec, M, N, 1);
	}

	/* transpose */

	template<typename DST_T, typename SRC_T> void transpose(DST_T* pDst, SRC_T* pSrc, int N) {
		for (int i = 0; i < N - 1; ++i) {
			for (int j = i + 1; j < N; ++j) {
				int ij = i * N + j;
				int ji = j * N + i;
				DST_T t = pSrc[ij];
				pDst[ij] = pSrc[ji];
				pDst[ji] = t;
			}
		}
	}

	template<typename DST_T> void transpose(DST_T* pMtx, int N) {
		transpose(pMtx, pMtx, N);
	}

	/* tuples */

	template<typename T>
	inline void tup_sqrt(T* pDst, const T* pSrc, const int iorg, const int iend) {
		for (int i = iorg; i <= iend; ++i) {
			pDst[i] = GWBase::tsqrt(pSrc[i]);
		}
	}

	template<typename T>
	inline void tup_sqrt(T* pDst, const T* pSrc, int n) {
		tup_sqrt(pDst, pSrc, 0, n - 1);
	}

	template<typename T>
	inline void tup_sqrt(T* pDst, int n) {
		tup_sqrt(pDst, pDst, 0, n - 1);
	}


	template<typename T>
	inline void tup_scl(T* pDst, const T* pSrc, const int iorg, const int iend, T s) {
		for (int i = iorg; i <= iend; ++i) {
			pDst[i] = pSrc[i] * s;
		}
	}

	template<typename T>
	inline void tup_scl(T* pDst, const T* pSrc, int n, T s) {
		tup_scl(pDst, pSrc, 0, n - 1, s);
	}

	template<typename T>
	inline void tup_scl(T* pDst, int n, float s) {
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
	inline void tup_rcp(T* pDst, const T* pSrc, int n) {
		tup_rcp(pDst, pSrc, 0, n - 1);
	}

	template<typename T>
	inline void tup_rcp(T* pDst, int n) {
		tup_rcp(pDst, pDst, 0, n - 1);
	}


	template<typename T>
	inline T tup_max(const T* pTup, const int iorg, const int iend) {
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

	template<typename T>
	inline T inner_row_vec(const T* pMtx, const int ncol, const int irow, const float* pVec, const int iorg, const int iend) {
		T s = T(0);
		const T* pRow = pMtx + (irow * ncol);
		for (int i = iorg; i <= iend; ++i) {
			s += pRow[i] * pVec[i];
		}
		return s;
	}

	template<typename T>
	inline T inner_row_vec(const T* pMtx, const int ncol, const int irow, const float* pVec) {
		return inner_row_vec(pMtx, ncol, irow, pVec, 0, ncol - 1);
	}

	template<typename T>
	inline T inner_col_vec(const T* pMtx, const int ncol, const int icol, const float* pVec, const int iorg, const int iend) {
		T s = T(0);
		for (int i = iorg; i <= iend; ++i) {
			const T* pCol = pMtx + (i * ncol) + icol;
			s += *pCol * pVec[i];
		}
		return s;
	}

	template<typename T>
	inline T inner_col_vec(const T* pMtx, const int ncol, const int icol, const float* pVec) {
		return inner_col_vec(pMtx, ncol, icol, pVec, 0, ncol - 1);
	}

	template<typename T>
	inline T inner_row_row(const T* pMtxA, const int ncolA, const int irowA, const T* pMtxB, const int ncolB, const int irowB, int iorg, int iend) {
		T s = T(0);
		const T* pRowA = pMtxA + (irowA * ncolA);
		const T* pRowB = pMtxB + (irowB * ncolB);
		for (int i = iorg; i <= iend; ++i) {
			s += pRowA[i] * pRowB[i];
		}
		return s;
	}

	template<typename T>
	inline T inner_row_row(const T* pMtxA, const int ncol, const int irowA, const T* pMtxB, const int irowB) {
		return inner_row_row(pMtxA, ncol, irowA, pMtxB, ncol, irowB, 0, ncol - 1);
	}

	template<typename T>
	inline T inner_row_row(const T* pMtx, const int ncol, const int irow1, const int irow2) {
		return inner_row_row(pMtx, ncol, irow1, pMtx, irow2);
	}

	template<typename T>
	inline T inner_row_col(const T* pMtxA, const int ncolA, const int irowA, const T* pMtxB, const int ncolB, const int icolB, int iorg, int iend) {
		T s = T(0);
		const T* pRowA = pMtxA + (irowA * ncolA);
		for (int i = iorg; i <= iend; ++i) {
			const T* pColB = pMtxB + (i * ncolB) + icolB;
			s += *pColB * pRowA[i];
		}
		return s;
	}

	template<typename T>
	inline T inner_col_col(const T* pMtxA, const int ncolA, const int icolA, const T* pMtxB, const int ncolB, const int icolB, int iorg, int iend) {
		T s = T(0);
		for (int i = iorg; i <= iend; ++i) {
			const T* pColA = pMtxA + (i * ncolA) + icolA;
			const T* pColB = pMtxB + (i * ncolB) + icolB;
			s += *pColA * *pColB;
		}
		return s;
	}


	/* solvers */

	template<typename T>
	inline bool lu_decomp(T* pLU, const T* pMtx, int n, T* pTmpVec /*[n]*/, int* pPerm /*[n]*/, int* pDetSgn = nullptr, T tolerance = T(1.0e-16)) {
		if (pLU != pMtx) {
			for (int i = 0; i < n * n; ++i) {
				pLU[i] = pMtx[i];
			}
		}
		for (int i = 0; i < n; ++i) {
			pTmpVec[i] = inner_row_row(pMtx, n, i, i);
		}
		tup_sqrt(pTmpVec, n);
		T eps = tup_max(pTmpVec, n) * tolerance;
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
				t = ::fabs(t) * pTmpVec[i];
				if (t > s) {
					s = t;
					idx = i;
				}
			}
			s = pLU[idx*n + j];
			if (::fabs(s) < eps) return false;
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
			s = 1.0f / s;
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

} // namespace
