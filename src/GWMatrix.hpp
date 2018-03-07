/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

namespace GWMatrix {
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
} // namespace
