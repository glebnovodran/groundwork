/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

template<typename T> class GWTransform {
public:
	T m[4][4];

	GWTransform() = default;
	GWVectorBase<T> apply_vec(GWVectorBase<T> v) const {
		GWTuple4<T> res;
		GWTuple4<T> vec;
		GWTuple::copy(vec, v);
		vec[3] = 0;
		GWMatrix::mul_mm(res.elems, vec.elems, &m[0][0], 1, 4, 4);
		return GWVectorBase<T>(res);
	}

	GWVectorBase<T> apply_pnt(GWVectorBase<T> v) const {
		GWTuple4<T> res;
		GWTuple4<T> vec;
		GWTuple::copy(vec, v);
		vec[3] = 1;
		GWMatrix::mul_mm(res.elems, vec.elems, &m[0][0], 1, 4, 4);
		return GWVectorBase<T>(res);
	}
};

typedef GWTransform<float> GWTransformF;
typedef GWTransform<double> GWTransformD;
