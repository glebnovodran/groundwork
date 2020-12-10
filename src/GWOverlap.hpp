/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

namespace GWOverlap {

	template<typename T> bool aabb_aabb(const GWVectorBase<T>& minA, const GWVectorBase<T>& maxA, const GWVectorBase<T>& minB, const GWVectorBase<T>& maxB) {
		if ((maxA.x < minB.x) || (minA.x > maxB.x)) { return false; }
		if ((maxA.y < minB.y) || (minA.y > maxB.y)) { return false; }
		if ((maxA.z < minB.z) || (minA.z > maxB.z)) { return false; }
		return true;
	}

	template<typename T> bool seg_aabb(const GWVectorBase<T>& p, const GWVectorBase<T>& q, const GWVectorBase<T>& min, const GWVectorBase<T>& max);
}