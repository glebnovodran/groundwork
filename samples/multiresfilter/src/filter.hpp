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

	NodeBands() : pRotG(nullptr), pRotL(nullptr), numFrames(0) {}
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
	bool mBuilt;
public:
	MotionBands() : mpNodes(nullptr), mpMot(nullptr), mNumNodes(0), mNumBands(0), mBuilt(false) {}

	void init(const GWMotion* pMot);
	void reset();
	void build();
	bool is_built() const { return mBuilt; }
	NodeBands* node_bands(uint32_t nodeId) const {
		return (nodeId < mNumNodes) ? &mpNodes[nodeId] : nullptr;
	}
	uint32_t num_bands() const { return mNumBands; }
	static uint32_t calc_number(uint32_t numFrames);

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
	void reset() { mBands.reset(); }
	void apply(uint32_t nodeId, GWVectorF* pMem, uint32_t numGains = 0, const float* pGains = nullptr);

	GWMotion* get_equalized() { return &mEqualizedMot; }
	const GWMotion* get_motion() const { return &mMot; }

	float get_gain(uint32_t band, uint32_t numGains = 0, const float* pGains = nullptr) const {
		if (pGains == nullptr) { return 1.0f; }
		return band < numGains ? pGains[band] : 1.0f;
	}

	uint32_t get_num_bands() const { return mBands.num_bands(); }
protected:
	void set_motion(const GWMotion& mot) {
		mMot.clone_from(mot);
		mEqualizedMot.clone_from(mot);
		mBands.init(&mMot);
	}
};

class MotionGains {
private:
	uint32_t mNumNodes;
	uint32_t mNumGains;
	float** mppGains;
public:
	MotionGains(uint32_t numNodes, uint32_t numGains) : mNumNodes(numNodes), mNumGains(numGains) {
		mppGains = new float*[numNodes]();
	}

	void load(std::string fpath);
	void apply_to(MotionEqualizer& equ);

	void set_gains(uint32_t nodeId, const float* pGains);
};
