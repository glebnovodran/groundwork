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

namespace GWBase {
	extern const long double pi;
}

namespace GWTuple {

template<typename TUPLE_T, typename... VAL_Ts> inline void set(TUPLE_T& dst, VAL_Ts... vals) {
	const int n = std::min(TUPLE_T::ELEMS_NUM, int(sizeof...(vals)));
	typename TUPLE_T::elem_t tmp[sizeof...(VAL_Ts)] = { TUPLE_T::elem_t(vals)... };
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

template<typename TUPLE_DST_T, typename TUPLE_SRC_T> inline void add(TUPLE_DST_T& dst, const TUPLE_SRC_T& src) {
	int n = std::min(TUPLE_DST_T::ELEMS_NUM, TUPLE_SRC_T::ELEMS_NUM);
	for (int i = 0; i < n; ++i) {
		dst.elems[i] = dst.elems[i] + TUPLE_DST_T::elem_t(src.elems[i]);
	}
}

template<typename TUPLE_DST_T, typename TUPLE_SRC_T> inline void sub(TUPLE_DST_T& dst, const TUPLE_SRC_T& src) {
	int n = std::min(TUPLE_DST_T::ELEMS_NUM, TUPLE_SRC_T::ELEMS_NUM);
	for (int i = 0; i < n; ++i) {
		dst.elems[i] = dst.elems[i] - TUPLE_DST_T::elem_t(src.elems[i]);
	}
}

template<typename TUPLE_DST_T, typename TUPLE_SRC_T> inline void mul(TUPLE_DST_T& dst, const TUPLE_SRC_T& src) {
	int n = std::min(TUPLE_DST_T::ELEMS_NUM, TUPLE_SRC_T::ELEMS_NUM);
	for (int i = 0; i < n; ++i) {
		dst.elems[i] = dst.elems[i] * TUPLE_DST_T::elem_t(src.elems[i]);
	}
}

template<typename TUPLE_DST_T, typename TUPLE_SRC_T> inline void div(TUPLE_DST_T& dst, const TUPLE_SRC_T& src) {
	int n = std::min(TUPLE_DST_T::ELEMS_NUM, TUPLE_SRC_T::ELEMS_NUM);
	for (int i = 0; i < n; ++i) {
		dst.elems[i] = dst.elems[i] / TUPLE_DST_T::elem_t(src.elems[i]);
	}
}

template<typename TUPLE_T, typename SCALAR_T> inline void scl(TUPLE_T& dst, const SCALAR_T& s) {
	int n = TUPLE_T::ELEMS_NUM;
	for (int i = 0; i < n; ++i) {
		dst.elems[i] = typename TUPLE_T::elem_t(dst.elems[i] * s);
	}
}

template<typename TUPLE_T> inline typename TUPLE_T::elem_t inner(TUPLE_T& a, const TUPLE_T& b) {
	int n = TUPLE_T::ELEMS_NUM;
	typename TUPLE_T::elem_t d = TUPLE_T::elem_t(0);
	for (int i = 0; i < n; ++i) {
		d += a.elems[i] * b.elems[i];
	}
	return d;
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

