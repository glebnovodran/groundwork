/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <cstdint>
#include <cmath>
#include <cstdarg>
#include <algorithm>

enum class GWTransformOrder : uint8_t {
	SRT = 0,
	STR = 1,
	RST = 2,
	RTS = 3,
	TSR = 4,
	TRS = 5
};

enum class GWRotationOrder : uint8_t {
	XYZ = 0,
	XZY = 1,
	YXZ = 2,
	YZX = 3,
	ZXY = 4,
	ZYX = 5
};

namespace GWBase {
	extern const long double pi;

	template<typename T> inline T radians(T deg) { return (T)(deg * (pi / 180)); }
	template<typename T> inline T degrees(T rad) { return (T)(rad * (180 / pi)); }
}

namespace GWTuple {

	template<typename TUPLE_T, typename... VAL_Ts> inline void set(TUPLE_T& dst, VAL_Ts... vals) {
		const int n = std::min(TUPLE_T::ELEMS_NUM, int(sizeof...(vals)));
		typename TUPLE_T::elem_t tmp[sizeof...(VAL_Ts)] = { typename TUPLE_T::elem_t(vals)... };
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = tmp[i];
		}
	}

	template<typename TUPLE_T, typename VAL_T> inline void fill(TUPLE_T& dst, VAL_T val) {
		const int n = TUPLE_T::ELEMS_NUM;
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = typename TUPLE_T::elem_t(val);
		}
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T> inline void copy(TUPLE_DST_T& dst, TUPLE_SRC_T src) {
		const int n = std::min(TUPLE_DST_T::ELEMS_NUM, TUPLE_SRC_T::ELEMS_NUM);
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = typename TUPLE_DST_T::elem_t(src.elems[i]);
		}
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC0_T, typename TUPLE_SRC1_T> inline void add(TUPLE_DST_T& dst, const TUPLE_SRC0_T& src0, const TUPLE_SRC1_T& src1) {
		int n = std::min(TUPLE_DST_T::ELEMS_NUM, std::min(TUPLE_SRC0_T::ELEMS_NUM, TUPLE_SRC1_T::ELEMS_NUM));
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = typename TUPLE_DST_T::elem_t(src0.elems[i] + src1.elems[i]);
		}
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T> inline void add(TUPLE_DST_T& dst, const TUPLE_SRC_T& src) {
		GWTuple::add(dst, dst, src);
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC0_T, typename TUPLE_SRC1_T> inline void sub(TUPLE_DST_T& dst, const TUPLE_SRC0_T& src0, const TUPLE_SRC1_T& src1) {
		int n = std::min(TUPLE_DST_T::ELEMS_NUM, std::min(TUPLE_SRC0_T::ELEMS_NUM, TUPLE_SRC1_T::ELEMS_NUM));
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = typename TUPLE_DST_T::elem_t(src0.elems[i] - src1.elems[i]);
		}
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T> inline void sub(TUPLE_DST_T& dst, const TUPLE_SRC_T& src) {
		GWTuple::sub(dst, dst, src);
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC0_T, typename TUPLE_SRC1_T> inline void mul(TUPLE_DST_T& dst, const TUPLE_SRC0_T& src0, const TUPLE_SRC1_T& src1) {
		int n = std::min(TUPLE_DST_T::ELEMS_NUM, std::min(TUPLE_SRC0_T::ELEMS_NUM, TUPLE_SRC1_T::ELEMS_NUM));
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = typename TUPLE_DST_T::elem_t(src0.elems[i] * src1.elems[i]);
		}
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T> inline void mul(TUPLE_DST_T& dst, const TUPLE_SRC_T& src) {
		GWTuple::mul(dst, dst, src);
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC0_T, typename TUPLE_SRC1_T> inline void div(TUPLE_DST_T& dst, const TUPLE_SRC0_T& src0, const TUPLE_SRC1_T& src1) {
		int n = std::min(TUPLE_DST_T::ELEMS_NUM, std::min(TUPLE_SRC0_T::ELEMS_NUM, TUPLE_SRC1_T::ELEMS_NUM));
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = typename TUPLE_DST_T::elem_t(src0.elems[i] / src1.elems[i]);
		}
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T> inline void div(TUPLE_DST_T& dst, const TUPLE_SRC_T& src) {
		GWTuple::div(dst, dst, src);
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T, typename SCALAR_T> inline void scl(TUPLE_DST_T& dst, const TUPLE_SRC_T& src, const SCALAR_T& s) {
		int n = TUPLE_DST_T::ELEMS_NUM;
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = typename TUPLE_DST_T::elem_t(src.elems[i] * s);
		}
	}

	template<typename TUPLE_DST_T, typename SCALAR_T> inline void scl(TUPLE_DST_T& dst, const SCALAR_T& s) {
		GWTuple::scl(dst, dst, s);
	}

	template<typename TUPLE_DST_T, typename TUPLE_SRC_T> inline void neg(TUPLE_DST_T& dst, const TUPLE_SRC_T& src) {
		int n = TUPLE_DST_T::ELEMS_NUM;
		for (int i = 0; i < n; ++i) {
			dst.elems[i] = - src.elems[i];
		}
	}

	template<typename TUPLE_DST_T> inline void neg(TUPLE_DST_T& dst) {
		GWTuple::neg(dst, dst);
	}

	template<typename TUPLE_SRC0_T, typename TUPLE_SRC1_T> inline typename TUPLE_SRC0_T::elem_t inner(const TUPLE_SRC0_T& a, const TUPLE_SRC1_T& b) {
		int n = std::min(TUPLE_SRC0_T::ELEMS_NUM, TUPLE_SRC1_T::ELEMS_NUM);
		typename TUPLE_SRC0_T::elem_t d = 0;
		for (int i = 0; i < n; ++i) {
			d += a.elems[i] * b.elems[i];
		}
		return d;
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
}

template<typename T> struct GWTuple2 {
	typedef T elem_t;
	static const int ELEMS_NUM = 2;
	union { struct { elem_t x, y; }; elem_t elems[ELEMS_NUM]; };
};

template<typename T> struct GWTuple3 {
	typedef T elem_t;
	static const int ELEMS_NUM = 3;
	union { struct { elem_t x, y, z; }; elem_t elems[ELEMS_NUM]; };
};

template<typename T> struct GWTuple4 {
	typedef T elem_t;
	static const int ELEMS_NUM = 4;
	union { struct { elem_t x, y, z, w; }; elem_t elems[ELEMS_NUM]; };
};

typedef GWTuple2<float> GWTuple2f;
typedef GWTuple3<float> GWTuple3f;
typedef GWTuple4<float> GWTuple4f;

typedef GWTuple2<double> GWTuple2d;
typedef GWTuple3<double> GWTuple3d;
typedef GWTuple4<double> GWTuple4d;

