/*
 * Motion multiresolution filtering
 * Author: Gleb Novodran <novodran@gmail.com>
 */
/*
struct NodeRawData {
	GWVectorF* pRot;
	GWVectorF* pTrn;
	GWVectorF* pScl;
	uint32_t len;
};

struct MotionRawData {
	NodeRawData* pNodeData;
	uint32_t numNodes;
};

struct BandData {
	MotionRawData* pMotData;
};
*/

struct NodeBands {
	GWVectorF* pRotG;
	GWVectorF* pRotL;
	uint32_t numFrames;

	inline GWVectorF* G(uint32_t idx) {
		return pRotG + idx * numFrames;
	};
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
	//NodeBands& node_bands(uint32_t idx) const { return pNodes[idx]; }
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

class MultiResFilter {
protected:
	GWMotion mMot;
	GWMotion mFilteredMot;
	MotionBands mBands;

public:

	MultiResFilter(const GWMotion& mot) : mBands() {
		set_motion(mot);
	}

	void build() { mBands.build(); }
	void apply(const float* pGains);

	GWMotion* get_filtered() { return &mFilteredMot; }
	const GWMotion* get_motion() const { return &mMot; }

protected:
	void set_motion(const GWMotion& mot) {
		mMot.clone_from(mot);
		mFilteredMot.clone_from(mot);
		mBands.init(&mMot);
	}

};
