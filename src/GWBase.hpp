/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <cstdint>
#include <cmath>
#include <cstdarg>
#include <algorithm>

#ifndef GW_NOINLINE
#	if defined(_MSC_VER)
#		define GW_NOINLINE __declspec(noinline)
#	elif defined(__GNUC__)
#		define GW_NOINLINE __attribute__((noinline))
#	else
#		define GW_NOINLINE
#	endif
#endif

#ifndef GW_FORCEINLINE
#	if defined(_MSC_VER)
#		define GW_FORCEINLINE __forceinline
#	elif defined(__GNUC__)
#		define GW_FORCEINLINE __inline__ __attribute__((__always_inline__))
#	else
#		define GW_FORCEINLINE inline
#	endif
#endif

enum class GWTransformOrder : uint8_t {
	SRT = 0,
	STR = 1,
	RST = 2,
	RTS = 3,
	TSR = 4,
	TRS = 5,
	MAX = TRS
};

enum class GWRotationOrder : uint8_t {
	XYZ = 0,
	XZY = 1,
	YXZ = 2,
	YZX = 3,
	ZXY = 4,
	ZYX = 5,
	MAX = ZYX
};

enum class GWTrackKind : uint8_t {
	ROT = 0,
	TRN = 1,
	SCL = 2,
	MAX = SCL
};

namespace GWBase {
	extern const long double pi;

	union Cvt32 {
		float f;
		int32_t i;
		uint32_t u;
		uint8_t b[4];
	};

	union Cvt64 {
		int64_t i;
		uint64_t u;
		double d;
		uint8_t b[8];
	};

	inline int32_t f32_ulp_diff(float a, float b) {
		if (a == b) return 0;
		const float e = 1.0e-6f;
		if (::fabs(a) < e && ::fabs(b) < e) return 0;
		Cvt32 ua;
		ua.f = a;
		Cvt32 ub;
		ub.f = b;
		const int32_t sm = 0x7FFFFFFF;
		if (ua.i < 0) ua.i &= sm;
		if (ub.i < 0) ub.i &= sm;
		return ua.u > ub.u ? ua.u - ub.u : ub.u - ua.u;
	}

	inline int64_t f64_ulp_diff(double a, double b, double nearz = 1.0e-12f) {
		if (a == b) return 0;
		if (::fabs(a) < nearz && ::fabs(b) < nearz) return 1;
		Cvt64 ua;
		ua.d = a;
		Cvt64 ub;
		ub.d = b;
		if (ua.i < 0) ua.i = (1ULL << 63) - ua.i;
		if (ub.i < 0) ub.i = (1ULL << 63) - ub.i;
		return ua.u > ub.u ? ua.u - ub.u : ub.u - ua.u;
	}

	template<typename T> inline bool almost_equal(T a, T b, T eps = T(0.0001f)) {
		return (T(::fabs(a - b)) <= eps);
	}

	static int64_t factorial(int64_t x) {
		int64_t res = 1;
		if (x > 1) {
			res = x;
			while (--x > 1) {
				res *= x;
			}
		}
		return res;
	}
	void half_to_float(float* pDst, const uint16_t* pSrc, int n);
	void float_to_half(uint16_t* pDst, const float* pSrc, int n);
	void vec_to_oct(float vx, float vy, float vz, float& ox, float& oy);
	void oct_to_vec(float ox, float oy, float& vx, float& vy, float& vz);

	template<typename T> inline T radians(T deg) { return (T)(deg * (pi / 180)); }
	template<typename T> inline T degrees(T rad) { return (T)(rad * (180 / pi)); }

	inline GWRotationOrder rord_from_int(int ival) {
		return ival > (int)GWRotationOrder::MAX ? GWRotationOrder::XYZ : GWRotationOrder(ival);
	}

	inline GWTransformOrder xord_from_int(int ival) {
		return ival > (int)GWTransformOrder::MAX ? GWTransformOrder::SRT : GWTransformOrder(ival);
	}

	inline GWRotationOrder rord_from_float(float val) {
		int ival = (int)val;
		return rord_from_int(ival);
	}
	inline GWTransformOrder xord_from_float(float val) {
		int ival = (int)val;
		return xord_from_int(ival);
	}

	inline GWTrackKind validate_track_kind(GWTrackKind kind) {
		return kind < GWTrackKind::MAX ? kind : GWTrackKind::ROT;
	}

	template<typename T> inline T clamp(T x, T lo, T hi) { return std::max(std::min(x, hi), lo); }
	template<typename T> inline T saturate(T x) { return clamp<T>(x, T(0), T(1)); }
	template<typename T> inline T lerp(T a, T b, T t) { return a + (b - a)*t; }

	template<typename T> inline T mod_pi(T rad) {
		T twoPi = T(2 * pi);
		rad = ::fmod(rad, twoPi);
		if (::fabs(rad) > pi) {
			rad = rad < T(0) ? twoPi + rad : rad - twoPi;
		}
		return rad;
	}

	template<typename T> inline T tsqrt(T x) { return ::sqrt(x); }
	inline float tsqrt(float x) { return ::sqrtf(x); } // GCC: force single-precision

	// Ranq1, Numerical Recipes 3d ed., chapter 3.7.1
	class Random {
	private:
		uint64_t mVal;
	public:
		Random(uint64_t seed = 0ULL) { set_seed(seed); }
		void set_seed(uint64_t seed) {
			mVal = seed ^ 4101842887655102017ULL;
			mVal = u64();
		}
		uint64_t u64() {
			mVal ^= mVal >> 21;
			mVal ^= mVal << 35;
			mVal ^= mVal >> 4;
			return mVal * 2685821657736338717ULL;
		}
	};

	void set_random_seed(uint64_t seed);
	uint64_t random_u64();

	struct StrHash {
		union {
			uint64_t val;
			struct {
				uint32_t len;
				uint32_t hash;
			};
		};
		StrHash() : val(0ULL) {}
		StrHash(const char* pStr) {
			calculate(pStr);
		}
		void calculate(const char* pStr);
	};

	inline bool operator == (const StrHash& hashA, const StrHash& hashB) {
		return hashA.val == hashB.val;
	}
}

namespace GWTuple {

	template<typename TUPLE_T, typename... VAL_Ts> inline void set(TUPLE_T& dst, VAL_Ts... vals) {
		const int n = std::min(TUPLE_T::ELEMS_NUM, int(sizeof...(vals)));
		typename TUPLE_T::elem_t tmp[sizeof...(VAL_Ts)] = { typename TUPLE_T::elem_t(vals)... };
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = tmp[i];
		}
	}

	template<typename TUPLE_T, typename VAL_T>
	inline void fill(TUPLE_T& dst, VAL_T val) {
		const int n = TUPLE_T::ELEMS_NUM;
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = typename TUPLE_T::elem_t(val);
		}
	}

	template<typename TUPLE_T, typename SCALAR_PTR_T>
	inline void load(TUPLE_T& dst, SCALAR_PTR_T pMem) {
		const int n = TUPLE_T::ELEMS_NUM;
		for (int i = 0; i < n; ++i) {
			dst[i] = pMem[i];
		}
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T>
	inline void copy(TUPLE_DST_T& dst, TUPLE_SRC_T src) {
		const int n = std::min(TUPLE_DST_T::ELEMS_NUM, TUPLE_SRC_T::ELEMS_NUM);
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = typename TUPLE_DST_T::elem_t(src.elems[i]);
		}
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T>
	inline void copy(TUPLE_DST_T& dst, TUPLE_SRC_T src, typename TUPLE_DST_T::elem_t padVal) {
		const int n = std::min(TUPLE_DST_T::ELEMS_NUM, TUPLE_SRC_T::ELEMS_NUM);
		const int padN = std::max(TUPLE_DST_T::ELEMS_NUM, TUPLE_SRC_T::ELEMS_NUM);
		int i;
		for (i = 0; i < n; ++i) {
			dst.elems[i] = typename TUPLE_DST_T::elem_t(src.elems[i]);
		}
		for (; i < padN; ++i) {
			dst.elems[i] = padVal;
		}
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC0_T, typename TUPLE_SRC1_T>
	inline void max(TUPLE_DST_T& dst, const TUPLE_SRC0_T& src0, const TUPLE_SRC1_T& src1) {
		const int n = std::min(TUPLE_DST_T::ELEMS_NUM, std::min(TUPLE_SRC0_T::ELEMS_NUM, TUPLE_SRC1_T::ELEMS_NUM));
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = std::max(src0.elems[i], src1.elems[i]);
		}
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC0_T, typename TUPLE_SRC1_T>
	inline void min(TUPLE_DST_T& dst, const TUPLE_SRC0_T& src0, const TUPLE_SRC1_T& src1) {
		const int n = std::min(TUPLE_DST_T::ELEMS_NUM, std::min(TUPLE_SRC0_T::ELEMS_NUM, TUPLE_SRC1_T::ELEMS_NUM));
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = std::min(src0.elems[i], src1.elems[i]);
		}
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T, typename SCALAR_T>
	inline void clip_low(TUPLE_DST_T& dst, const TUPLE_SRC_T& src, SCALAR_T val) {
		const int n = std::min(TUPLE_DST_T::ELEMS_NUM, TUPLE_SRC_T::ELEMS_NUM);
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = std::max(src.elems[i], val);
		}
	}

	template<typename TUPLE_DST_T, typename SCALAR_T>
	inline void clip_low(TUPLE_DST_T& dst, SCALAR_T val) {
		GWTuple::clip_low(dst, dst, val);
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T, typename SCALAR_T>
	inline void clip_high(TUPLE_DST_T& dst, const TUPLE_SRC_T& src, SCALAR_T val) {
		const int n = std::min(TUPLE_DST_T::ELEMS_NUM, TUPLE_SRC_T::ELEMS_NUM);
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = std::min(src.elems[i], val);
		}
	}

	template<typename TUPLE_DST_T, typename SCALAR_T>
	inline void clip_high(TUPLE_DST_T& dst, SCALAR_T val) {
		GWTuple::clip_high(dst, dst, val);
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T, typename SCALAR_T>
	inline void clamp(TUPLE_DST_T& dst, const TUPLE_SRC_T& src, SCALAR_T low, SCALAR_T high) {
		const int n = std::min(TUPLE_DST_T::ELEMS_NUM, TUPLE_SRC_T::ELEMS_NUM);
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = std::max(std::min(src.elems[i], high), low);
		}
	}

	template<typename TUPLE_DST_T, typename SCALAR_T>
	inline void clamp(TUPLE_DST_T& dst, SCALAR_T low, SCALAR_T high) {
		GWTuple::clamp(dst, dst, low, high);
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T>
	inline void saturate(TUPLE_DST_T& dst, const TUPLE_SRC_T& src) {
		clamp(dst, src, 0.0f, 1.0f);
	}

	template<typename TUPLE_DST_T>
	inline void saturate(TUPLE_DST_T& dst) {
		clamp(dst, dst, 0.0f, 1.0f);
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC0_T, typename TUPLE_SRC1_T>
	inline void add(TUPLE_DST_T& dst, const TUPLE_SRC0_T& src0, const TUPLE_SRC1_T& src1) {
		const int n = std::min(TUPLE_DST_T::ELEMS_NUM, std::min(TUPLE_SRC0_T::ELEMS_NUM, TUPLE_SRC1_T::ELEMS_NUM));
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = typename TUPLE_DST_T::elem_t(src0.elems[i] + src1.elems[i]);
		}
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T>
	inline void add(TUPLE_DST_T& dst, const TUPLE_SRC_T& src) {
		GWTuple::add(dst, dst, src);
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC0_T, typename TUPLE_SRC1_T>
	inline void sub(TUPLE_DST_T& dst, const TUPLE_SRC0_T& src0, const TUPLE_SRC1_T& src1) {
		const int n = std::min(TUPLE_DST_T::ELEMS_NUM, std::min(TUPLE_SRC0_T::ELEMS_NUM, TUPLE_SRC1_T::ELEMS_NUM));
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = typename TUPLE_DST_T::elem_t(src0.elems[i] - src1.elems[i]);
		}
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T>
	inline void sub(TUPLE_DST_T& dst, const TUPLE_SRC_T& src) {
		GWTuple::sub(dst, dst, src);
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC0_T, typename TUPLE_SRC1_T>
	inline void mul(TUPLE_DST_T& dst, const TUPLE_SRC0_T& src0, const TUPLE_SRC1_T& src1) {
		const int n = std::min(TUPLE_DST_T::ELEMS_NUM, std::min(TUPLE_SRC0_T::ELEMS_NUM, TUPLE_SRC1_T::ELEMS_NUM));
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = typename TUPLE_DST_T::elem_t(src0.elems[i] * src1.elems[i]);
		}
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T>
	inline void mul(TUPLE_DST_T& dst, const TUPLE_SRC_T& src) {
		GWTuple::mul(dst, dst, src);
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC0_T, typename TUPLE_SRC1_T>
	inline void div(TUPLE_DST_T& dst, const TUPLE_SRC0_T& src0, const TUPLE_SRC1_T& src1) {
		const int n = std::min(TUPLE_DST_T::ELEMS_NUM, std::min(TUPLE_SRC0_T::ELEMS_NUM, TUPLE_SRC1_T::ELEMS_NUM));
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = typename TUPLE_DST_T::elem_t(src0.elems[i] / src1.elems[i]);
		}
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T>
	inline void div(TUPLE_DST_T& dst, const TUPLE_SRC_T& src) {
		GWTuple::div(dst, dst, src);
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T, typename SCALAR_T>
	inline void scl(TUPLE_DST_T& dst, const TUPLE_SRC_T& src, const SCALAR_T& s) {
		const int n = std::min(TUPLE_DST_T::ELEMS_NUM, TUPLE_SRC_T::ELEMS_NUM);
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = typename TUPLE_DST_T::elem_t(src.elems[i] * s);
		}
	}

	template<typename TUPLE_DST_T, typename SCALAR_T>
	inline void scl(TUPLE_DST_T& dst, const SCALAR_T& s) {
		GWTuple::scl(dst, dst, s);
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T>
	inline void neg(TUPLE_DST_T& dst, const TUPLE_SRC_T& src) {
		const int n = std::min(TUPLE_DST_T::ELEMS_NUM, TUPLE_SRC_T::ELEMS_NUM);
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = - src.elems[i];
		}
	}

	template<typename TUPLE_DST_T> inline void neg(TUPLE_DST_T& dst) {
		GWTuple::neg(dst, dst);
	}
	// Add scaled: y = ax + y
	template<typename TUPLE_DST_T, typename TUPLE_SRC_T, typename SCALAR_T>
	inline void add_scaled(TUPLE_DST_T& y, const TUPLE_SRC_T& x, SCALAR_T a) {
		const int n = std::min(TUPLE_DST_T::ELEMS_NUM, TUPLE_SRC_T::ELEMS_NUM);
		for (int i = 0; i < n; ++i) {
			y.elems[i] += a * x.elems[i];
		}
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC0_T, typename TUPLE_SRC1_T, typename T>
	inline void lerp(TUPLE_DST_T& dst, const TUPLE_SRC0_T& a, const TUPLE_SRC1_T& b, T t) {
		const int n = std::min(TUPLE_DST_T::ELEMS_NUM, std::min(TUPLE_SRC0_T::ELEMS_NUM, TUPLE_SRC1_T::ELEMS_NUM));
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = typename TUPLE_DST_T::elem_t( GWBase::lerp(a.elems[i], b.elems[i], t));
		}
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T, typename T>
	inline void lerp(TUPLE_DST_T& a, const TUPLE_SRC_T& b, T t) {
		GWTuple::lerp(a, a, b, t);
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC0_T, typename TUPLE_SRC1_T, typename T>
	inline void lerp_fma(TUPLE_DST_T& dst, const TUPLE_SRC0_T& a, const TUPLE_SRC1_T& b, T t) {
		const int n = std::min(TUPLE_DST_T::ELEMS_NUM, std::min(TUPLE_SRC0_T::ELEMS_NUM, TUPLE_SRC1_T::ELEMS_NUM));
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = typename TUPLE_DST_T::elem_t(::fma(b.elems[i] - a.elems[i], t, a.elems[i]));
		}
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T, typename T>
	inline void lerp_fma(TUPLE_DST_T& a, const TUPLE_SRC_T& b, T t) {
		GWTuple::lerp_fma(a, a, b, t);
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T>
	inline void abs(TUPLE_DST_T& dst, const TUPLE_SRC_T& src) {
		const int n = std::min(TUPLE_DST_T::ELEMS_NUM, TUPLE_SRC_T::ELEMS_NUM);
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = ::fabs(src.elems[i]);
		}
	}

	template<typename TUPLE_DST_T> inline void abs(TUPLE_DST_T& dst) {
		GWTuple::abs(dst, dst);
	}

	template<typename TUPLE_SRC0_T, typename TUPLE_SRC1_T>
	inline typename TUPLE_SRC0_T::elem_t inner(const TUPLE_SRC0_T& a, const TUPLE_SRC1_T& b) {
		int n = std::min(TUPLE_SRC0_T::ELEMS_NUM, TUPLE_SRC1_T::ELEMS_NUM);
		typename TUPLE_SRC0_T::elem_t d = 0;
		for (int i = 0; i < n; ++i) {
			d += a.elems[i] * b.elems[i];
		}
		return d;
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T>
	inline void sqrt(TUPLE_DST_T& dst, const TUPLE_SRC_T& src) {
		const int n = std::min(TUPLE_DST_T::ELEMS_NUM, TUPLE_SRC_T::ELEMS_NUM);
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = GWBase::tsqrt(src.elems[i]);
		}
	}

	template<typename TUPLE_DST_T> inline void sqrt(TUPLE_DST_T& dst) {
		GWTuple::sqrt(dst, dst);
	}

	template<typename TUPLE_T> inline typename TUPLE_T::elem_t sum(const TUPLE_T& v) {
		int n = TUPLE_T::ELEMS_NUM;
		typename TUPLE_T::elem_t s = TUPLE_T::elem_t(0);
		for (int i = 1; i < n; ++i) {
			s += v.elems[i];
		}
		return s;
	}

	template<typename TUPLE_T> inline typename TUPLE_T::elem_t min_elem(const TUPLE_T& v) {
		int n = TUPLE_T::ELEMS_NUM;
		typename TUPLE_T::elem_t min = v.elems[0];
		for (int i = 1; i < n; ++i) {
			if (min > v.elems[i]) { min = v.elems[i]; }
		}
		return min;
	}

	template<typename TUPLE_T> inline typename TUPLE_T::elem_t max_elem(const TUPLE_T& v) {
		int n = TUPLE_T::ELEMS_NUM;
		typename TUPLE_T::elem_t max = v.elems[0];
		for (int i = 1; i < n; ++i) {
			if (max < v.elems[i]) { max = v.elems[i]; }
		}
		return max;
	}

	template<typename TUPLE_T> inline typename TUPLE_T::elem_t min_abs_elem(const TUPLE_T& v) {
		int n = TUPLE_T::ELEMS_NUM;
		typename TUPLE_T::elem_t minVal = ::fabs(v.elems[0]);
		for (int i = 1; i < n; ++i) {
			typename TUPLE_T::elem_t absVal = ::fabs(v.elems[i]);
			minVal = std::min(minVal, absVal);
		}
		return minVal;
	}

	template<typename TUPLE_T> inline typename TUPLE_T::elem_t max_abs_elem(const TUPLE_T& v) {
		int n = TUPLE_T::ELEMS_NUM;
		typename TUPLE_T::elem_t maxVal = ::fabs(v.elems[0]);
		for (int i = 1; i < n; ++i) {
			typename TUPLE_T::elem_t absVal = ::fabs(v.elems[i]);
			maxVal = std::max(maxVal, absVal);
		}
		return maxVal;
	}

	template<typename TUPLE_T> inline typename TUPLE_T::elem_t magnitude_fast(const TUPLE_T& v) {
		typename TUPLE_T::elem_t magSq = GWTuple::inner(v, v);
		return ::sqrt(magSq);
	}

	// Mike Day "Vector length and normalization difficulties"
	template<typename TUPLE_T> inline typename TUPLE_T::elem_t magnitude(const TUPLE_T& v) {
		using namespace GWTuple;
		typename TUPLE_T::elem_t maxAbsElem = max_abs_elem(v);
		typename TUPLE_T::elem_t res = 0;
		if (maxAbsElem > 0) {
			TUPLE_T tuple = v;
			typename TUPLE_T::elem_t divisor = 1 / maxAbsElem;
			scl(tuple, divisor);
			res = magnitude_fast(tuple) * maxAbsElem;
		}
		return res;
	}

	// Mike Day "Vector length and normalization difficulties"
	template<typename TUPLE_T> inline void normalize(TUPLE_T& dst, typename TUPLE_T::elem_t* pMag = nullptr) {
		typename TUPLE_T::elem_t maxAbsElem = max_abs_elem(dst);
		typename TUPLE_T::elem_t mag = 0;
		if (maxAbsElem > 0) {
			typename TUPLE_T::elem_t divisor = 1 / maxAbsElem;
			scl(dst, divisor);
			mag = magnitude_fast(dst);
			scl(dst, 1 / mag);
		}

		if (pMag) { *pMag = mag * maxAbsElem; }
	}

	template<typename TUPLE_T> inline void normalize(TUPLE_T& dst, const TUPLE_T& src, typename TUPLE_T::elem_t* pMag = nullptr) {
		copy(dst, src);
		normalize(dst, pMag);
	}

	template<typename TUPLE_T> inline void normalize_fast(TUPLE_T& v) {
		typename TUPLE_T::elem_t mag = magnitude_fast(v);
		if (mag > 0) {
			scl(v, 1 / mag);
		}
	}
	template<typename TUPLE_T> inline void normalize_fast(TUPLE_T& dst, const TUPLE_T& src) {
		copy(dst, src);
		normalize_fast(dst);
	}

	template<typename TUPLE_T> inline void calc_bbox(const TUPLE_T* pData, uint32_t len, TUPLE_T& minVal, TUPLE_T& maxVal) {
		for (uint32_t i = 0; i < len; ++i) {
			if (i == 0) {
				minVal = pData[i];
				maxVal = pData[i];
			} else {
				GWTuple::max(maxVal, pData[i], maxVal);
				GWTuple::min(minVal, pData[i], minVal);
			}
		}
	}
	template<typename TUPLE_SRC_T, typename SCALAR_T>
	inline bool compare(const TUPLE_SRC_T& a, const TUPLE_SRC_T& b, SCALAR_T eps) {
		const int n = TUPLE_SRC_T::ELEMS_NUM;
		bool res = true;
		for (int i = 0; i < n; ++i) {
			if (::fabs(a.elems[i] - b.elems[i]) > eps) { res = false; break; }
		}
		return res;
	}
} // GWTuple

template<typename T> struct GWTuple2 {
	typedef T elem_t;
	static const int ELEMS_NUM = 2;
	union { struct { elem_t x, y; }; elem_t elems[ELEMS_NUM]; };

	T operator [](size_t i) const { return elems[i]; }
	T& operator [](size_t i) { return elems[i]; }
	template<typename TUPLE_T> GWTuple2& operator = (const TUPLE_T& tuple) {
		GWTuple::copy(*this, tuple);
		return *this;
	}
};

template<typename T> struct GWTuple3 {
	typedef T elem_t;
	static const int ELEMS_NUM = 3;
	union { struct { elem_t x, y, z; }; elem_t elems[ELEMS_NUM]; };

	T operator [](size_t i) const { return elems[i]; }
	T& operator [](size_t i) { return elems[i]; }

	template<typename TUPLE_T> GWTuple3& operator = (const TUPLE_T& tuple) {
		GWTuple::copy(*this, tuple);
		return *this;
	}
};

template<typename T> struct GWTuple4 {
	typedef T elem_t;
	static const int ELEMS_NUM = 4;
	union { struct { elem_t x, y, z, w; }; elem_t elems[ELEMS_NUM]; };

	T operator [](size_t i) const { return elems[i]; }
	T& operator [](size_t i) { return elems[i]; }

	template<typename TUPLE_T> GWTuple4& operator = (const TUPLE_T& tuple) {
		GWTuple::copy(*this, tuple);
		return *this;
	}
};

template<typename T> struct GWColorTuple3 {
	typedef T elem_t;
	static const int ELEMS_NUM = 3;
	union { struct { elem_t r, g, b; }; elem_t elems[ELEMS_NUM]; };

	T operator [](size_t i) const { return elems[i]; }
	T& operator [](size_t i) { return elems[i]; }

	template<typename TUPLE_T> GWColorTuple3& operator = (const TUPLE_T& tuple) {
		GWTuple::copy(*this, tuple);
		return *this;
	}
};

template<typename T> struct GWColorTuple4 {
	typedef T elem_t;
	static const int ELEMS_NUM = 4;
	union { struct { elem_t r, g, b, a; }; elem_t elems[ELEMS_NUM]; };

	T operator [](size_t i) const { return elems[i]; }
	T& operator [](size_t i) { return elems[i]; }

	template<typename TUPLE_T> GWColorTuple4& operator = (const TUPLE_T& tuple) {
		GWTuple::copy(*this, tuple);
		return *this;
	}
};

struct GWHalf {
	uint16_t x;
	float get() const {
		float f;
		GWBase::half_to_float(&f, &x, 1);
		return f;
	}
	void set(float f) {
		GWBase::float_to_half(&x, &f, 1);
	}
};

typedef GWTuple2<float> GWTuple2f;
typedef GWTuple3<float> GWTuple3f;
typedef GWTuple4<float> GWTuple4f;

typedef GWTuple2<double> GWTuple2d;
typedef GWTuple3<double> GWTuple3d;
typedef GWTuple4<double> GWTuple4d;

typedef GWColorTuple3<float> GWColorTuple3f;
typedef GWColorTuple4<float> GWColorTuple4f;

typedef GWTuple2<int> GWTuple2i;
typedef GWTuple3<int> GWTuple3i;
typedef GWTuple4<int> GWTuple4i;

typedef GWTuple2<uint32_t> GWTuple2u;
typedef GWTuple3<uint32_t> GWTuple3u;
typedef GWTuple4<uint32_t> GWTuple4u;

typedef GWTuple2<GWHalf> GWTuple2h;
typedef GWTuple3<GWHalf> GWTuple3h;
typedef GWTuple4<GWHalf> GWTuple4h;

struct GWHalf2 : GWTuple2h {
	GWTuple2f get() const {
		GWTuple2f tuple;
		GWBase::half_to_float((float*)tuple.elems, (uint16_t*)elems, ELEMS_NUM);
		return tuple;
	}
	void set(const GWTuple2f& tuple) {
		GWBase::float_to_half((uint16_t*)elems, (float*)tuple.elems, ELEMS_NUM);
	}
};

struct GWHalf3 : GWTuple3h {
	GWTuple3f get() const {
		GWTuple3f tuple;
		GWBase::half_to_float((float*)tuple.elems, (uint16_t*)elems, ELEMS_NUM);
		return tuple;
	}
	void set(const GWTuple3f& tuple) {
		GWBase::float_to_half((uint16_t*)elems, (float*)tuple.elems, ELEMS_NUM);
	}
};

struct GWHalf4 : GWTuple4h {
	GWTuple4f get() const {
		GWTuple4f tuple;
		GWBase::half_to_float((float*)tuple.elems, (uint16_t*)elems, ELEMS_NUM);
		return tuple;
	}
	void set(const GWTuple4f& tuple) {
		GWBase::float_to_half((uint16_t*)elems, (float*)tuple.elems, ELEMS_NUM);
	}
};

