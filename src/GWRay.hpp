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
		nOrig(orig), mDir(dir) {}
	GWVectorBase<T> origin() const { return mOrig; }
	GWVectorBase<T> direction() const { return mDir; }
	GWVectorBase<T> at(T t) const { return mOrig + t * mDir; }

	inline void from_asimuth_elevation(T azimuth, T elevation) {
		T sinA = ::sin(azimuth);
		T cosA = ::cos(azimuth);
		T sinE = ::sin(elevation);
		T cosE = ::cos(elevation);
		mDir.x = cosA * sinE;
		mDir.y = cosE;
		mDir.z = sinA * sinE;
	}

	inline void from_polar_uv(T u, T v) {
		float azimuth = u * T(2) * GWBase::pi;
		float elevation = (v - T(1)) * GWBase::pi;
		from_asimuth_elevation(azimuth, elevation);
	}

};
