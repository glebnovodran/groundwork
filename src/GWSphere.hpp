/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */
template<typename T> struct GWSphereBase {
	GWVectorBase<T> c;
	T r;
	GWSphereBase() : c(T(0)), r(T(0)) {};
	GWSphereBase(GWTuple3<T> center, T rad) {
		this->c = center;
		this->r = rad;
	}
	GWSphereBase(T x, T y, T z, T rad) {
		c.x = x; c.y = y; c.z = z;
		r = rad;
	}

	inline void enclose(const GWVectorBase<T>& pnt) {
		GWVectorBase<T> d = pnt - c;
		T distSq = d.length_sq();
		if (distSq > r*r) {
			T dist = ::sqrt(distSq);
			T newR = (r + dist) * T(0.5);
			T s = (newR - r) / dist;
			r = newR;
			c += (d * s);
		
		}
	}

	void set_zero() {
		c.fill(0.0f);
		r = 0.0f;
	}

	void ritter(const GWVectorBase<T>* pPts, int numPts);
};
/*
namespace GWSphere {
	// Ritter's bounding sphere
	template<typename T> GWSphereBase<T> ritter(GWVectorBase<T>* pPts, int numPts);
}
*/
typedef GWSphereBase<float> GWSphereF;
typedef GWSphereBase<double> GWSphereD;
