struct GWCamera {
	GWTransformF mViewMtx;
	GWTransformF mProjMtx;
	GWTransformF mViewProjMtx;
	GWVectorF mPos;
	GWVectorF mTgt;
	GWVectorF mUp;
	int mWidth;
	int mHeight;
	float mFOVY;
	float mNear;
	float mFar;

	float get_aspect() const {
		return GWBase::div0(float(mWidth), float(mHeight));
	}

	void update(const GWVectorF& pos, const GWVectorF& tgt, const GWVectorF& up, const float fovy, const float near, const float far) {
		mPos = pos;
		mTgt = tgt;
		mUp = up;
		mFOVY = fovy;
		mNear = near;
		mFar = far;

		mViewMtx.make_view(pos, tgt, up);
		mProjMtx.make_projection(mFOVY, get_aspect(), mNear, mFar);
		mViewProjMtx = GWXform::concatenate(mViewMtx, mProjMtx);
	}
};