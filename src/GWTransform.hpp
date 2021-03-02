/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

namespace GWXform {

// http://graphics.pixar.com/library/OrthonormalB/paper.pdf
template <typename T>
inline void ortho_from_Y_axis(const GWVectorBase<T>& ny, GWVectorBase<T>& x, GWVectorBase<T>& z) {
	T sign = ny.z < T(0) ? T(-1) :  T(1);
	const T a = -GWBase::rcp0(sign - ny.z);
	const T b = ny.x * ny.y * a;
	GWTuple::set(x, T(1) + sign * ny.x*ny.x * a, sign * b, sign * ny.x);
	GWTuple::set(z, b, sign + ny.y*ny.y * a, ny.y);
}

} // namespace GWXform

template<typename T> class GWTransform {
public:
	T m[4][4];

	T* as_tptr() {
		return reinterpret_cast<T*>(m);
	}
	const T* as_tptr() const {
		return reinterpret_cast<const T*>(m);
	}

	void set_zero() {
		T* pData = as_tptr();
		for (int i = 0; i < 16; ++i) { pData[i] = T(0); }
	}

	void set_identity() { make_scaling(1, 1, 1); }

	void set_row(uint32_t idx, const GWVectorBase<T>& v, T pad = 0) {
		for (int i = 0; i < 3; ++i) {
			m[idx][i] = v[i];
		}
		m[idx][3] = pad;
	}

	void set_translation(const GWVectorBase<T>& v) {
		set_row(3, v, 1);
	}

	inline void set_translation(T tx, T ty, T tz) {
		m[3][0] = tx;
		m[3][1] = ty;
		m[3][2] = tz;
		m[3][3] = T(1);
	}

	GWVectorBase<T> get_translation() const {
		GWVectorBase<T> trn(m[3][0], m[3][1], m[3][2]);
		return trn;
	}

	void make_translation(T tx, T ty, T tz) {
		set_identity();
		set_translation(tx, ty, tz);
	}

	void make_translation(const GWVectorBase<T>& trn) {
		make_translation(trn.x, trn.y, trn.z);
	}

	void copy_translation(const GWTransform<T>& xform) {
		set_translation(xform.get_translation());
	}

	inline void set_scaling(T sx, T sy, T sz) {
		m[0][0] = sx;
		m[1][1] = sy;
		m[2][2] = sz;
		m[3][3] = T(1);
	}

	inline void set_scaling(T s) {
		for (int i = 0; i < 3; ++i) {
			m[i][i] = s;
		}
		m[3][3] = T(1);
	}

	GWVectorBase<T> get_scaling() const {
		GWVectorBase<T> scl(m[0][0], m[1][1], m[2][2]);
		return scl;
	}

	void make_scaling(T sx, T sy, T sz) {
		set_zero();
		set_scaling(sx, sy, sz);
	}

	void make_scaling(const GWVectorBase<T>& scl) {
		make_scaling(scl.x, scl.y, scl.z);
	}

	void make_scaling(T s) {
		set_zero();
		set_scaling(s);
	}

	void make_rx(T rx) {
		T s = std::sin(rx);
		T c = std::cos(rx);
		set_identity();
		m[1][1] = c;
		m[1][2] = s;
		m[2][1] = -s;
		m[2][2] = c;
	}
	void make_ry(T ry) {
		T s = std::sin(ry);
		T c = std::cos(ry);
		set_identity();
		m[0][0] = c;
		m[0][2] = -s;
		m[2][0] = s;
		m[2][2] = c;
	}
	void make_rz(T rz) {
		T s = std::sin(rz);
		T c = std::cos(rz);
		set_identity();
		m[0][0] = c;
		m[0][1] = s;
		m[1][0] = -s;
		m[1][1] = c;
	}
	void make_rotation(T rx, T ry, T rz, GWRotationOrder order = GWRotationOrder::XYZ);
	void make_rotation(const GWQuaternionBase<T>& q) {
		set_row(0, q.calc_axis_x());
		set_row(1, q.calc_axis_y());
		set_row(2, q.calc_axis_z());
		set_row(3, GWVectorBase<T>(0), T(1));
	}

	void make_deg_rx(T degX) { make_rx(GWBase::radians(degX)); }
	void make_deg_ry(T degY) { make_ry(GWBase::radians(degY)); }
	void make_deg_rz(T degZ) { make_rz(GWBase::radians(degZ)); }
	void make_deg_rotation(T degX, T degY, T degZ, GWRotationOrder order = GWRotationOrder::XYZ) {
		T rx = GWBase::radians(degX);
		T ry = GWBase::radians(degY);
		T rz = GWBase::radians(degZ);
		make_rotation(rx, ry, rz, order);
	}

	GWQuaternionBase<T> get_rotation() const {
		return GWUnitQuaternion::from_transform(*this, 4, true);
	}

	void make_transform(const GWQuaternionBase<T>& rot, const GWVectorBase<T>& trn, const GWVectorBase<T>& scl, GWTransformOrder order = GWTransformOrder::SRT);

	void make_projection(T fovY, T aspect, T znear, T zfar);

	void make_view(const GWVectorBase<T>& pos, const GWVectorBase<T>& tgt, const GWVectorBase<T>& upVec = GWVectorBase<T>(0,1,0)) {
		GWVectorBase<T> dir = (tgt - pos);
		dir.normalize();
		GWVectorBase<T> side = GWVector::cross(upVec, dir);
		side.normalize();
		GWVectorBase<T> up = GWVector::cross(side, dir);

		set_row(0, -side, T(0));
		set_row(1, -up, T(0));
		set_row(2, -dir, T(0));
		transpose_sr();
		set_translation(calc_vec(-pos));
	}

	void from_Y_axis(const GWVectorBase<T>& y) {
		GWVectorBase<T> x,z;
		GWXform::ortho_from_Y_axis(y, x, z);
		set_row(0, x, T(0));
		set_row(1, y, T(0));
		set_row(2, z, T(0));
		set_translation(T(0), T(0), T(0));
	}

	void apply(const GWTransform& parent) {
		GWTransform res;
		GWMatrix::mul_mm(res.as_tptr(), as_tptr(), parent.as_tptr(), 4, 4, 4);
		(*this) = res;
	}

	void invert(const GWTransform& m0) {
		(*this) = m0.get_inverted();
	}
	void invert() { invert(*this); }

	GWTransform get_inverted() const {
		GWTransform inv;
		int idxc[4];
		int idxr[4];
		int pivot[4];
		T* pInv = inv.as_tptr();

		GWMatrix::gj_solve(pInv, idxc, idxr, pivot, as_tptr(), 4);
		GWMatrix::gj_inv(pInv, pInv, 4, idxc, idxr);
		return inv;
	}

	void invert_fast(const GWTransform& m0) {
		(*this) = m0.get_inverted_fast();
	}
	void invert_fast() { invert_fast(*this); }

	GWTransform get_inverted_fast() const;

	void transpose(const GWTransform& x0) {
		GWMatrix::transpose(as_tptr(), x0.as_tptr(), 4);
	}
	void transpose() {
		GWMatrix::transpose(as_tptr(), 4);
	}

	void transpose_sr(const GWTransform& m0);
	void transpose_sr() { transpose_sr(*this); }

	GWVectorBase<T> calc_vec(const GWVectorBase<T>& v) const {
		GWTuple4<T> res;
		GWMatrix::mul_vm(res.elems, v.elems, as_tptr(), 3, 4);
		return GWVectorBase<T>(res);
	}

	GWVectorBase<T> calc_pnt(const GWVectorBase<T>& v) const {
		GWTuple4<T> res;
		GWTuple4<T> vec;
		GWTuple::copy(vec, v);
		vec[3] = 1;
		GWMatrix::mul_vm(res.elems, vec.elems, as_tptr(), 4, 4);
		return GWVectorBase<T>(res);
	}

	inline T determinant() const {
		T a0 = m[0][0] * m[1][1] - m[0][1] * m[1][0];
		T a1 = m[0][0] * m[1][2] - m[0][2] * m[1][0];
		T a2 = m[0][0] * m[1][3] - m[0][3] * m[1][0];
		T a3 = m[0][1] * m[1][2] - m[0][2] * m[1][1];
		T a4 = m[0][1] * m[1][3] - m[0][3] * m[1][1];
		T a5 = m[0][2] * m[1][3] - m[0][3] * m[1][2];

		T b0 = m[2][0] * m[3][1] - m[2][1] * m[3][0];
		T b1 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
		T b2 = m[2][0] * m[3][3] - m[2][3] * m[3][0];
		T b3 = m[2][1] * m[3][2] - m[2][2] * m[3][1];
		T b4 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
		T b5 = m[2][2] * m[3][3] - m[2][3] * m[3][2];

		return a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;
	}

	GWTuple4<T>* as_tuple(uint32_t idx) const {
		GWTuple4<T>* pTuple = nullptr;
		if (idx < 4) {
			pTuple = (GWTuple4<T>*)m[idx];
		}
		return pTuple;
	}

	bool almost_equal(const GWTransform& xform, T eps) const {
		for (uint32_t i = 0; i < 4; ++i) {
			GWTuple4<T>* pTuple = as_tuple(i);
			if (!GWTuple::almost_equal(*as_tuple(i),
				*xform.as_tuple(i),
				eps)) {
				return false;
			}
		}
		return true;
	}
};

typedef GWTransform<float> GWTransformF;
typedef GWTransform<double> GWTransformD;

template<typename T> class GWTransform3x4 {
public:
	T m[3][4];
	T* as_tptr() {
		return reinterpret_cast<T*>(m);
	}
	const T* as_tptr() const {
		return reinterpret_cast<const T*>(m);
	}

	void set_zero() {
		T* pData = as_tptr();
		for (int i = 0; i < 12; ++i) { pData[i] = T(0); }
	}

	void set_identity() { make_scaling(1, 1, 1); }

	void set_column(uint32_t idx, const GWVectorBase<T>& v) {
		for (int i = 0; i < 3; ++i) {
			m[i][idx] = v[i];
		}
	}

	void set_translation(const GWVectorBase<T>& v) {
		set_column(3, v);
	}

	inline void set_translation(T tx, T ty, T tz) {
		m[0][3] = tx;
		m[1][3] = ty;
		m[2][3] = tz;
	}

	GWVectorBase<T> get_translation() const {
		GWVectorBase<T> trn(m[0][3], m[1][3], m[2][3]);
		return trn;
	}

	void make_translation(T tx, T ty, T tz) {
		set_identity();
		set_translation(tx, ty, tz);
	}

	void copy_translation(const GWTransform<T>& xform) {
		set_translation(xform.get_translation());
	}

	void copy_translation(const GWTransform3x4<T>& xform34) {
		set_translation(xform34.get_translation());
	}

	inline void set_scaling(T sx, T sy, T sz) {
		m[0][0] = sx;
		m[1][1] = sy;
		m[2][2] = sz;
	}

	inline void set_scaling(T s) {
		for (int i = 0; i < 3; ++i) {
			m[i][i] = s;
		}
	}

	GWVectorBase<T> get_scaling() const {
		GWVectorBase<T> scl(m[0][0], m[1][1], m[2][2]);
		return scl;
	}

	void make_scaling(T sx, T sy, T sz) {
		set_zero();
		set_scaling(sx, sy, sz);
	}

	void make_scaling(const GWVectorBase<T>& scl) {
		make_scaling(scl.x, scl.y, scl.z);
	}

	void make_scaling(T s) {
		set_zero();
		set_scaling(s);
	}

	//transposed comparing to 4x4
	void make_rx(T rx) {
		T s = std::sin(rx);
		T c = std::cos(rx);
		set_identity();
		m[1][1] = c;
		m[2][1] = s;
		m[1][2] = -s;
		m[2][2] = c;
	}
	void make_ry(T ry) {
		T s = std::sin(ry);
		T c = std::cos(ry);
		set_identity();
		m[0][0] = c;
		m[2][0] = -s;
		m[0][2] = s;
		m[2][2] = c;
	}
	void make_rz(T rz) {
		T s = std::sin(rz);
		T c = std::cos(rz);
		set_identity();
		m[0][0] = c;
		m[1][0] = s;
		m[0][1] = -s;
		m[1][1] = c;
	}

	void make_deg_rx(T degX) { make_rx(GWBase::radians(degX)); }
	void make_deg_ry(T degY) { make_ry(GWBase::radians(degY)); }
	void make_deg_rz(T degZ) { make_rz(GWBase::radians(degZ)); }

	void make_rotation(const GWQuaternionBase<T>& q) {
		set_column(0, q.calc_axis_x());
		set_column(1, q.calc_axis_y());
		set_column(2, q.calc_axis_z());
		set_column(3, GWVectorBase<T>(0));
	}
	void make_rotation(T rx, T ry, T rz, GWRotationOrder order = GWRotationOrder::XYZ) {
		GWQuaternionBase<T> q;
		q.set_radians(rx, ry, rz, order);
		make_rotation(q);
	}
	void make_deg_rotation(T degX, T degY, T degZ, GWRotationOrder order = GWRotationOrder::XYZ) {
		GWQuaternionBase<T> q;
		q.set_degrees(degX, degY, degZ, order);
		make_rotation(q);
	}

	GWQuaternionBase<T> get_rotation() const {
		return GWUnitQuaternion::from_transform(as_tptr(), 4, false);
	}

	void make_transform(const GWQuaternionBase<T>& rot, const GWVectorBase<T>& trn, const GWVectorBase<T>& scl, GWTransformOrder order = GWTransformOrder::SRT) {
		GWTransform<T> xform;
		xform.make_transform(rot, trn, scl, order);
		xform.transpose();
		GWMatrix::copy(as_tptr(), xform.as_tptr(), 3, 4);
	}

	void from_Y_axis(const GWVectorBase<T>& y) {
		GWVectorBase<T> x,z;
		GWXform::ortho_from_Y_axis(y, x, z);
		set_column(0, x);
		set_column(1, y);
		set_column(2, z);
		set_translation(T(0), T(0), T(0));
	}


	GWVectorBase<T> calc_vec(const GWVectorBase<T>&v) const {
		GWTuple4<T> res;
		GWTuple4<T> vec;
		GWTuple::copy(vec, v, 0);
		GWMatrix::mul_mv(res.elems, vec.elems, as_tptr(), 3, 4);
		return GWVectorBase<T>(res);
	}

	GWVectorBase<T> calc_pnt(const GWVectorBase<T>&v) const {
		GWTuple4<T> res;
		GWTuple4<T> vec;
		GWTuple::copy(vec, v);
		vec[3] = 1;
		GWMatrix::mul_mv(res.elems, vec.elems, as_tptr(), 3, 4);
		return GWVectorBase<T>(res);
	}
};

typedef GWTransform3x4<float> GWTransform3x4F;
typedef GWTransform3x4<double> GWTransform3x4D;

namespace GWXformCvt {

template <typename T> inline GWTransform3x4<T> get_3x4(const GWTransform<T>& xform) {
	GWTransform<T> tmp;
	GWTransform3x4<T> res;
	tmp.transpose(xform);
	GWMatrix::copy(res.as_tptr(), tmp.as_tptr(), 3, 4);
	return res;
}

template <typename T> inline GWTransform<T> get_4x4(const GWTransform3x4<T>& x34) {
	GWTransform<T> res;
	T* pDst = res.as_tptr();
	const T* pSrc = x34.as_tptr();
	GWMatrix::copy(pDst, pSrc, 3, 4);
	res.set_row(3, GWVectorBase<T>(T(0)), T(1));
	res.transpose();
	return res;
}

} // namespace GWXformCvt

namespace GWXform {

template <typename T>
inline GWTransform<T> concatenate(const GWTransform<T>& child, const GWTransform<T>& parent) {
	GWTransform<T> res;
	GWMatrix::mul_mm(res.as_tptr(), child.as_tptr(), parent.as_tptr(), 4, 4, 4);
	return res;
}

template <typename T>
inline GWTransform3x4<T> concatenate(const GWTransform3x4<T>& child, const GWTransform3x4<T>& parent) {
	return GWXformCvt::get_3x4(concatenate(GWXformCvt::get_4x4(child), GWXformCvt::get_4x4(parent)));
}

} // namespace GWXform
