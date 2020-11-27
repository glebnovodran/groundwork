/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

namespace GWIntersect {
	template<typename T> bool seg_tri_ccw_alt(const GWVectorBase<T>& p, const GWVectorBase<T>& q,
		const GWVectorBase<T>& a, const GWVectorBase<T>& b, const GWVectorBase<T>& c, GWVectorBase<T>* pHitPos, GWVectorBase<T>* pHitDir) {

		GWVectorBase<T> ab = b - a;
		GWVectorBase<T> ac = c - a;
		GWVectorBase<T> n = GWVector::cross(ab, ac);
		GWVectorBase<T> qp = p - q;
		T d = qp.dot(n);
		if (d <= T(0.0f)) { return false; }

		GWVectorBase<T> ap = p - a;
		T t = ap.dot(n);
		if ((t < T(0.0f)) || (t > d)) { return false; }
		GWVectorBase<T> e = GWVector::cross(qp, ap);
		T u, v, w;
		v = ac.dot(e);
		if ((v < T(0.0f)) || (v > d)) { return false; }
		w = -ab.dot(e);
		if ((w < T(0.0f)) || ((w + v) > d)) { return false; }
		t = t / d;
		if (pHitPos) {
			*pHitPos = p + t * (q - p);
		}
		if (pHitDir) {
			*pHitDir = n; // non-normalized direction
		}
		return true;
	}

	template<typename T> bool seg_tri_ccw(const GWVectorBase<T>& p, const GWVectorBase<T>& q,
		const GWVectorBase<T>& v0, const GWVectorBase<T>& v1, const GWVectorBase<T>& v2, GWVectorBase<T>* pHitPos, GWVectorBase<T>* pHitNrm) {

		GWVectorBase<T> edge[3];
		GWVectorBase<T> vp[3];
		GWVectorBase<T> dir = q - p;
		edge[0] = v1 - v0;
		vp[0] = p - v0;
		GWVectorBase<T> n = GWVector::cross(edge[0], v2 - v0);
		n.normalize();
		T d0 = vp[0].dot(n);
		T d1 = (q - v0).dot(n);

		if (d0*d1 > T(0.0f) || (d0 == 0.0f && d1 == T(0.0f))) return false;

		edge[1] = v2 - v1;
		edge[2] = v0 - v2;
		vp[1] = p - v1;
		vp[2] = p - v2;

		if (GWVector::triple(edge[0], dir, vp[0]) < T(0.0f)) { return false; }
		if (GWVector::triple(edge[1], dir, vp[1]) < T(0.0f)) { return false; }
		if (GWVector::triple(edge[2], dir, vp[2]) < T(0.0f)) { return false; }

		T d = dir.dot(n);
		T t;
		if ((d == T(0.0f)) || (d0 == T(0.0f))) {
			t = T(0.0f);
		} else {
			t = -d0/d;
		}
		if (t > T(1.0f) || t < T(0.0f)) return false;
		if (pHitPos) {
			*pHitPos = p + t * dir;
		}
		if (pHitNrm) {
			*pHitNrm = n;
		}
		return true;
	}
}