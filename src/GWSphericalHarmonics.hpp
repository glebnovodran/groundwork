/*
 * Spherical Harmonics
 * Author: Gleb Novodran <novodran@gmail.com>
 */

// The initial code was generated by
// https://github.com/schaban/crossdata_gfx/blob/master/tool/shgen.py
// and then reformulated in the array form presented here

namespace GWSH {
	template <typename T> GW_FORCEINLINE
		void vec_project_i(T* pCoefs, const T x[], const T y[], const T z[], const int N = 1) {
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
	void vec_project(T* pCoefs, const T x[], const T y[], const T z[], const int N = 1) {
		vec_project_i(pCoefs, x, y, z, N);
	}

	template <typename T> inline
	void calc_phong_weights(T* pWgt, T s, T scl) {
		if (pWgt) {
			for (int i = 0; i < 3; ++i) {
				pWgt[i] = ::exp(float(-i * i) / (T(2) * s)) * scl;
			}
		}
	}

	template <typename T> inline
	void calc_irradiance_weights(T* pWgt, T scl) {
		if (pWgt) {
			std::fill_n(pWgt, 3, T(0));
			pWgt[0] = scl;
			pWgt[1] = scl / T(1.5f);
			pWgt[2] = scl / T(4.0f);
		}
	}

}
template<typename T> class GWSHCoeffsBase {
protected:
	static const int ORDER = 3;

	static constexpr int get_idx(int l, int m) { return l*(l + 1) + m; }
	static constexpr int get_coef_num(int order) { return order*order; }
	static constexpr int N = get_coef_num(ORDER);

	GWColorTuple3<T> mCoef[N];
public:
	GWSHCoeffsBase() = default;

	inline T* as_raw() { return reinterpret_cast<T*>(mCoef); }
	void load(T* pData);

	GWColorTuple3<T> operator ()(int l, int m) const { return mCoef[get_idx(l, m)]; }
	GWColorTuple3<T>& operator ()(int l, int m) { return mCoef[get_idx(l, m)]; }

	GWColorTuple3<T> operator ()(int i) const { return mCoef[i]; }
	GWColorTuple3<T>& operator ()(int i) { return mCoef[i]; }

	void clear() {
		T* pData = as_raw();
		for (int i = 0; i < N * 3; ++i) { pData[i] = T(0); }
	}

	void scl(T s) {
		T* pData = as_raw();
		for (int i = 0; i < N * 3; ++i) { pData[i] *= s; }
	}

	void apply_weights(T* pWgt) {
		const int NUM_WEIGHTS = 3;
		for (int l = 0; l < ORDER; ++l) {
			int i0 = get_coef_num(l);
			int i1 = get_coef_num(l + 1);
			float w = pWgt[l];
			for (int i = i0; i < i1; ++i) {
				GWTuple::scl(mCoef[i], w);
			}
		}
	}

	void lerp(GWSHCoeffsBase<T>& coefA, GWSHCoeffsBase<T>& coefB, T t);

	void calc_const(const GWColorF& clr);
	void calc_dir(const GWColorF& clr, const GWVectorBase<T>& dir);
	void calc_pano(const GWImage* pImg);

	GWColorTuple3<T> synthesize(T x, T y, T z) const {
		T nx[1] = { x };
		T ny[1] = { y };
		T nz[1] = { z };
		T dirCoefs[N];

		GWSH::vec_project<T>(dirCoefs, nx, ny, nz, 1);

		GWColorTuple3<T> clr;
		GWTuple::fill(clr, T(0));
		for (int i = 0; i < N; ++i) {
			GWTuple::add_scaled(clr, mCoef[i], dirCoefs[i]);
		}

		return clr;
	}

	void synth_pano(GWImage* pImg) const;
};

typedef GWSHCoeffsBase<float> GWSHCoeffsF;
typedef GWSHCoeffsBase<double> GWSHCoeffsD;
