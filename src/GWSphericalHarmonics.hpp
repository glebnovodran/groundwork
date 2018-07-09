template <typename T> GW_FORCEINLINE
void sh3_eval_i(T* pCoefs, const T x[], const T y[], const T z[], const int N = 1) {
	for (int i = 0; i < N; ++i) {
		pCoefs[i] = T(0.28209479177387814);
	}
	T* pDst = &pCoefs[N];
	for (int i = 0; i < N; ++i) {
		pDst[i] = y[i] * T(-0.48860251190292);
	}
	pDst = &pCoefs[2 * N];
	for (int i = 0; i < N; ++i) {
		pDst[i] = z[i] * T(0.4886025119029199);
	}
	pDst = &pCoefs[3 * N];
	for (int i = 0; i < N; ++i) {
		pDst[i] = x[i] * T(-0.48860251190292);
	}
	pDst = &pCoefs[4 * N];
	for (int i = 0; i < N; ++i) {
		pDst[i] = x[i] * y[i];
	}
	for (int i = 0; i < N; ++i) {
		pDst[i] *= T(2) * T(0.5462742152960395);
	}
	pDst = &pCoefs[5 * N];
	for (int i = 0; i < N; ++i) {
		pDst[i] = z[i] * y[i];
	}
	for (int i = 0; i < N; ++i) {
		pDst[i] *= T(-1.092548430592079);
	}
	pDst = &pCoefs[7 * N];
	for (int i = 0; i < N; ++i) {
		pDst[i] = z[i] * x[i];
	}
	for (int i = 0; i < N; ++i) {
		pDst[i] *= T(-1.092548430592079);
	}
	pDst = &pCoefs[6 * N];
	for (int i = 0; i < N; ++i) {
		pDst[i] = z[i] * z[i];
	}
	for (int i = 0; i < N; ++i) {
		pDst[i] *= T(0.9461746957575601);
	}
	for (int i = 0; i < N; ++i) {
		pDst[i] -= T(0.31539156525252005);
	}
	pDst = &pCoefs[8 * N];
	for (int i = 0; i < N; ++i) {
		pDst[i] = x[i] * x[i];
	}
	for (int i = 0; i < N; ++i) {
		pDst[i] -= y[i] * y[i];
	}
	for (int i = 0; i < N; ++i) {
		pDst[i] *= T(0.5462742152960395);
	}
}

template <typename T> inline
void sh3_eval(T* pCoefs, const T x[], const T y[], const T z[], const int N = 1) {
	sh3_eval_i(pCoefs, x, y, z, N);
}

