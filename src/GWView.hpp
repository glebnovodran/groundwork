

class GWScreenIfc {
public:
	virtual int get_width() { return 0; }
	virtual int get_height() { return 0; }
};

struct GWCamera {
	GWTransformF mViewMtx;
	GWTransformF mProjMtx;
	GWTransformF mViewProjMtx;
	GWTransformF mInvViewMtx;
	GWTransformF mInvProjMtx;
	GWTransformF mInvViewProj;
	GWVectorF mPos;
	GWVectorF mTgt;
	GWVectorF mUp;
	GWScreenIfc* mpScreenIfc;
	float mFOVY;
	float mNear;
	float mFar;

	void init(GWScreenIfc* pIfc) {
		mpScreenIfc = pIfc;
	}

	void set_view(const GWVectorF& pos, const GWVectorF& tgt, const GWVectorF& up = GWVectorF(0.0f, 1.0f,0.0f)) {
		mPos = pos;
		mTgt = tgt;
		mUp = up;
	}
	void set_view_range(const float near, const float far) {
		mNear = near;
		mFar = far;
	}
	void set_FOVY(const float fovY) {
		mFOVY = fovY;
	}

	float get_aspect() const {
		int width = mpScreenIfc->get_width();
		int height = mpScreenIfc->get_height();
		return GWBase::div0(float(width), float(height));
	}
	void update() {
		mViewMtx.make_view(mPos, mTgt, mUp);
		mProjMtx.make_projection(mFOVY, get_aspect(), mNear, mFar);
		mViewProjMtx = GWXform::concatenate(mViewMtx, mProjMtx);

		mInvProjMtx = mProjMtx.get_inverted();
		mInvViewProj = mViewProjMtx.get_inverted();
	}

	GWVectorF get_dir() const {
		GWVectorF dir = mTgt - mPos;
		dir.normalize();
		return dir;
	}

	GWVectorF get_ray_dir(const float u, const float v) {
		float du = GWBase::fit(u, 0.0f, 1.0f, -1.0f, 1.0f);
		float dv = GWBase::fit(v, 0.0f, 1.0f, -1.0f, 1.0f);
		GWVectorF dir(du, dv, 1.0f);
		dir = mInvProjMtx.calc_pnt(dir);
		dir.normalize();
		dir = mInvViewMtx.calc_vec(dir);
		return dir;
	}

};
