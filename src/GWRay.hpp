/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

template<typename T> class GWRayBase {
protected:
	GWVectorBase<T> mOrig;
	GWVectorBase<T> mDir;

public:
	GWRayBase() = default;
	GWRayBase(const GWVectorBase<T>& orig, const GWVectorBase<T>& dir) :
		mOrig(orig), mDir(dir) {}
	GWVectorBase<T> origin() const { return mOrig; }
	GWVectorBase<T> direction() const { return mDir; }
	GWVectorBase<T> at(T t) const { return mOrig + t * mDir; }

	inline void from_asimuth_inclination(T azimuth, T inclination) {
		T sinA = std::sin(azimuth);
		T cosA = std::cos(azimuth);
		T sinI = std::sin(inclination);
		T cosI = std::cos(inclination);
		mDir.x = cosA * sinI;
		mDir.y = cosI;
		mDir.z = sinA * sinI;
	}

	inline void from_polar_uv(T u, T v) {
		float azimuth = u * T(2) * GWBase::pi;
		float inclination = (v - T(1)) * GWBase::pi;
		from_asimuth_inclination(azimuth, inclination);
	}

};

typedef GWRayBase<float> GWRayF;
typedef GWRayBase<double> GWRayD;