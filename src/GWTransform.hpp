/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

template<typename T> class GWQuaternionBase;

template<typename T> class GWTransform {
public:
	T m[4][4];

	void set_zero() {
		T* pData = &m[0][0];
		for (int i = 0; i < 16; ++i) {
			pData[i] = 0.0f;
		}
	}

	void set_identity() {
		make_scaling(1, 1, 1);
	}

	void set_row(uint32_t idx, const GWVectorBase<T>& v, T pad = 0) {
		for (int i = 0; i < 3; ++i) {
			m[idx][i] = v[i];
		}
		m[idx][3] = pad;
	}

	void set_translation(const GWVectorBase<T>& v) {
		set_row(3, v, 1);
	}

	void set_translation(T tx, T ty, T tz) {
		m[3][0] = tx;
		m[3][1] = ty;
		m[3][2] = tz;
		m[3][3] = T(1);
	}

	void make_translation(T tx, T ty, T tz) {
		set_zero();
		set_translation(tx, ty, tz);
	}

	void make_translation(const GWVectorBase<T>& trn) {
		T* pData = &m[0][0];
		make_translation(trn.x, trn.y, trn.z);
	}

	void make_scaling(T sx, T sy, T sz) {
		T* pData = &m[0][0];
		set_zero();
		m[0][0] = sx;
		m[1][1] = sy;
		m[2][2] = sz;
		m[3][3] = T(1);
	}

	void make_scaling(const GWVectorBase<T>& scl) {
		T* pData = &m[0][0];
		make_scaling(scl.x, scl.y, scl.z);
	}

	void make_transform(const GWQuaternionBase<T>& rot, const GWVectorBase<T>& trn, const GWVectorBase<T>& scl, GWTransformOrder order = GWTransformOrder::SRT);

	void mul(const GWTransform& m0, const GWTransform& m1) {
		GWTransform res;
		GWMatrix::mul_mm(&res.m[0][0], &m0.m[0][0], &m1.m[0][0], 4, 4, 4);
		(*this) = res;
	}
	void mul(const GWTransform& m) { mul(*this, m); }

	void invert(const GWTransform& m);
	void invert();

	void transpose(const GWTransform& m);
	void transpose();

	GWVectorBase<T> calc_vec(const GWVectorBase<T>& v) const {
		GWTuple4<T> res;
		GWMatrix::mul_vm(res.elems, v.elems, &m[0][0], 3, 4);
		return GWVectorBase<T>(res);
	}

	GWVectorBase<T> calc_pnt(const GWVectorBase<T>& v) const {
		GWTuple4<T> res;
		GWTuple4<T> vec;
		GWTuple::copy(vec, v);
		vec[3] = 1;
		GWMatrix::mul_vm(res.elems, vec.elems, &m[0][0], 4, 4);
		return GWVectorBase<T>(res);
	}
};

template<typename T> inline GWTransform<T> operator * (const GWTransform<T>& m0, const GWTransform<T>& m1) {
	GWTransform<T> m;
	m.mul(m0, m1);
	return m;
}

typedef GWTransform<float> GWTransformF;
typedef GWTransform<double> GWTransformD;
