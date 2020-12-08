/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

namespace GWIntersect {

	// Ericson, Real-Time Collision Detection 2nd ed., p. 191
	template<typename T> bool seg_tri_ccw(const GWVectorBase<T>& p, const GWVectorBase<T>& q,
		const GWVectorBase<T>& a, const GWVectorBase<T>& b, const GWVectorBase<T>& c, GWVectorBase<T>* pHitPos, GWVectorBase<T>* pHitNrm) {

		GWVectorBase<T> ab = b - a;
		GWVectorBase<T> ac = c - a;
		GWVectorBase<T> n = GWVector::cross(ab, ac);
		GWVectorBase<T> qp = p - q;
		T d = qp.dot(n);
		if (d <= T(0)) { return false; }

		GWVectorBase<T> ap = p - a;
		T t = ap.dot(n);
		if ((t < T(0)) || (t > d)) { return false; }
		GWVectorBase<T> e = GWVector::cross(qp, ap);
		T u, v, w;
		v = ac.dot(e);
		if ((v < T(0)) || (v > d)) { return false; }
		w = -ab.dot(e);
		if ((w < T(0)) || ((v + w) > d)) { return false; }
		t = t / d;
		if (pHitPos) {
			*pHitPos = p + t * (q - p);
		}
		if (pHitNrm) {
			n.normalize();
			*pHitNrm = n;
		}
		return true;
	}

}