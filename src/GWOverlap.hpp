/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

namespace GWOverlap {

	template<typename T> bool aabb_aabb_overlap(const GWVectorBase<T>& minA, const GWVectorBase<T>& maxA, const GWVectorBase<T>& minB, const GWVectorBase<T>& maxB) {
		if ((maxA.x < minB.x) || (minA.x > maxB.x)) { return false; }
		if ((maxA.y < minB.y) || (minA.y > maxB.y)) { return false; }
		if ((maxA.z < minB.z) || (minA.z > maxB.z)) { return false; }

		return true;
	}
}