/*
 * Motion multiresolution filtering
 * https://www.researchgate.net/publication/2325084_Motion_Signal_Processing
 * alt link: http://mrl.snu.ac.kr/courses/CourseDataDrivenAnimation/readings/BRUDRLIN.PDF
 * Author: Gleb Novodran <novodran@gmail.com>
 */

struct NodeBands {
	GWVectorF* pRotG;
	GWVectorF* pRotL;
	uint32_t numFrames;

	// Gaussian pyramid level
	inline GWVectorF* G(uint32_t idx) {
		return pRotG + idx * numFrames;
	};
	// Laplacian pyramid level
	inline GWVectorF* L(uint32_t idx) { return pRotL + idx * numFrames; };

	inline GWVectorF* G(uint32_t idx, int fno) {
		fno = fno < 0 ? fno + numFrames : fno;
		fno %= numFrames;
		return (G(idx) + fno);
	};

	inline GWVectorF* L(uint32_t idx, int fno) {
		return (L(idx) + fno);
	}
};

class MotionBands {
protected:
	NodeBands* mpNodes;
	const GWMotion* mpMot;
	uint32_t mNumNodes;
	uint32_t mNumBands;

public:
	MotionBands() : mpNodes(nullptr), mpMot(nullptr), mNumNodes(0), mNumBands(0) {}

	void init(const GWMotion* pMot);
	static uint32_t calc_number(uint32_t numFrames);
	void build();

protected:
	void copy_g0();
	void build_low_pass(uint32_t lvl);
	void build_band_pass(uint32_t lvl);
};

class MotionEqualizer {
protected:
	GWMotion mMot;
	GWMotion mEqualizedMot;
	MotionBands mBands;

public:

	MotionEqualizer(const GWMotion& mot) : mBands() {
		set_motion(mot);
	}

	void build() { mBands.build(); }
	void apply(uint32_t nodeId, const float* pGains);

	GWMotion* get_filtered() { return &mEqualizedMot; }
	const GWMotion* get_motion() const { return &mMot; }

protected:
	void set_motion(const GWMotion& mot) {
		mMot.clone_from(mot);
		mEqualizedMot.clone_from(mot);
		mBands.init(&mMot);
	}

};