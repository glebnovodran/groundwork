class GWColorF {
public:
	GWColorTuple4f mRGBA;

	float luma() const {
		return mRGBA.r*0.299f + mRGBA.g*0.587f + mRGBA.b*0.114f;
	}

	float luminance() const {
		return mRGBA.r*0.212671f + mRGBA.g*0.71516f + mRGBA.b*0.072169f;
	}

	float average() const {
		return (mRGBA.r + mRGBA.g + mRGBA.b) / 3.0f;
	}

	void add(const GWColorF& c) { GWTuple::add(mRGBA, c.mRGBA); }
	void add(const GWColorF& c0, const GWColorF& c1) { GWTuple::add(mRGBA, c0.mRGBA, c1.mRGBA); }

	void sub(const GWColorF& c) { GWTuple::sub(mRGBA, c.mRGBA); }
	void sub(const GWColorF& c0, const GWColorF& c1) { GWTuple::sub(mRGBA, c0.mRGBA, c1.mRGBA); }

	void mul(const GWColorF& c) { GWTuple::mul(mRGBA, c.mRGBA); }
	void mul(const GWColorF& c0, const GWColorF& c1) { GWTuple::mul(mRGBA, c0.mRGBA, c1.mRGBA); }

	void scl(const GWColorF& c, float s) { GWTuple::scl(mRGBA, c.mRGBA, s); }
	void scl(float s) { GWTuple::scl(mRGBA, s); }

	void scl_rgb(const GWColorF& c, float s) {
		mRGBA.r = c.mRGBA.r * s;
		mRGBA.g = c.mRGBA.g * s;
		mRGBA.b = c.mRGBA.b * s;
	}
	void scl_rgb(float s) {
		mRGBA.r *= s;
		mRGBA.g *= s;
		mRGBA.b *= s;
	}
};