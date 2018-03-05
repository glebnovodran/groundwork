/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

template<typename T> class GWTransform {
public:
	T m[4][4];

	void set_row(uint32_t idx, const GWVectorBase<T>& v, T pad = 0) {
		for (int i = 0; i < 3; ++i) {
			m[idx][i] = v[i];
		}
		m[idx][3] = pad;
	}

	GWVectorBase<T> calc_vec(const GWVectorBase<T>& v) const {
		GWTuple4<T> res;
		GWTuple4<T> vec;
		GWTuple::copy(vec, v);
		vec[3] = 0;
		GWMatrix::mul_vm(res.elems, vec.elems, &m[0][0], 4);
		return GWVectorBase<T>(res);
	}

	GWVectorBase<T> calc_pnt(const GWVectorBase<T>& v) const {
		GWTuple4<T> res;
		GWTuple4<T> vec;
		GWTuple::copy(vec, v);
		vec[3] = 1;
		GWMatrix::mul_vm(res.elems, vec.elems, &m[0][0], 4);
		return GWVectorBase<T>(res);
	}
};

typedef GWTransform<float> GWTransformF;
typedef GWTransform<double> GWTransformD;
