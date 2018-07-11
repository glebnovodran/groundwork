/*
 * Spherical Harmonics
 * Author: Gleb Novodran <novodran@gmail.com>
 */

// The initial code was generated by
// https://github.com/schaban/crossdata_gfx/blob/master/tool/shgen.py
// and then reformulated in the array form presented here
template <typename T> GW_FORCEINLINE
void sh3_vec_project_i(T* pCoefs, const T x[], const T y[], const T z[], const int N = 1) {
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
void sh3_vec_project(T* pCoefs, const T x[], const T y[], const T z[], const int N = 1) {
	sh3_vec_project_i(pCoefs, x, y, z, N);
}

template<typename T> class GWSHCoeffsBase {
protected:
	static const int N = 9;
	GWColorTuple3<T> mCoef[N];

	static inline int get_idx(int l, int m) { return l*(l + 1) + m; }
public:
	GWSHCoeffsBase() = default;

	inline T* as_raw() const { return reinterpret_cast<T>(mCoeffs); }
	void load(T* pData);

	GWColorTuple3<T> operator ()(int l, int m) const { return mCoef[get_idx(l, m)]; }
	GWColorTuple3<T>& operator ()(int l, int m) const { return mCoef[get_idx(l, m)]; }

	void clear() {
		T* pData = as_raw();
		for (int i = 0; i < N * 3; ++i) { pData[i] = T(0); }
	}

	void scl(T s) {
		T* pData = as_raw();
		for (int i = 0; i < N * 3; ++i) { pData[i] *= s; }
	}

	void lerp(GWSHCoeffsBase<T>& coefA, GWSHCoeffsBase<T>& coefB, T t);

	void calc_const(GWColorF& clr);
	void calc_dir(GWColorF& clr, GWVectorBase<T>& dir);
};

typedef GWSHCoeffsBase<float> GWSHCoeffsF;
typedef GWSHCoeffsBase<double> GWSHCoeffsD;
