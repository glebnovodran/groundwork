/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

namespace GWMatrix {
	// MxP = MxN * NxP
	template<typename DST_T, typename SRC1_T, typename SRC2_T>
	inline void mul_mm(DST_T* pDst, const SRC1_T* pSrc1, const SRC2_T* pSrc2, int m, int n, int p) {
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

} // namespace
