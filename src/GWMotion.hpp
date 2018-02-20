/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

class GWMotion {
public:

	struct TrackInfo {
		float* pFrmData;
		GWVectorF minVal;
		GWVectorF maxVal;
		uint32_t numFrames;
		GWTrackKind kind;
		uint8_t dataMask;
		uint8_t srcMask;
//		uint8_t stride;

		TrackInfo() : pFrmData(nullptr), minVal(0), maxVal(0), numFrames(0),
			kind(GWTrackKind::ROT), dataMask(0), srcMask(0) {}

		void reset() {
			if (pFrmData != nullptr) { delete[] pFrmData; }
		}

		int get_stride() const {
			int val = 0;
			for (int i = 0; i < 3; ++i) {
				if (dataMask & (1 << i)) { ++val; }
			}
			return val;
		}

		void create_from_raw(GWVectorF* pRawData, uint32_t len, uint8_t srcMask);
	};

	class Track {
	private:
		const GWMotion* mpMot;
		uint32_t mTrackKind;
		uint32_t mNodeId;

		Track(const GWMotion* pMot, uint32_t trackKind = 0, uint32_t nodeId = 0) : mpMot(pMot), mTrackKind(trackKind), mNodeId(nodeId) {}
	public:
		void get_val(int fno, GWVectorF& res); // ==> mpMot->get_val(mNodeId, mTrackKind, fno)
		void eval(float frame, GWVectorF& res);
		void eval_quat(float frame, GWVectorF& res);

		friend class GWMotion;
	};

	struct NodeInfo {
		TrackInfo* pRotTrk;
		TrackInfo* pTrnTrk;
		TrackInfo* pSclTrk;
		GWTransformOrder* pXOrd; // add sizes
		GWRotationOrder* pROrd;
		uint32_t numFrames;
		GWTransformOrder defXOrd;
		GWRotationOrder defROrd;

		NodeInfo() : pRotTrk(nullptr), pTrnTrk(nullptr), pSclTrk(nullptr), pXOrd(nullptr), pROrd(nullptr),
			numFrames(0), defXOrd(GWTransformOrder::RST), defROrd(GWRotationOrder::XYZ) {}

		GWRotationOrder get_rord(uint32_t frameNo) const {
			return pROrd == nullptr? GWRotationOrder::XYZ : pROrd[frameNo % numFrames];
		}
		// get track
	};

	class Node {
	private:
		const GWMotion* mpMot;
		uint32_t mNodeId;

		Node(const GWMotion* pMot, uint32_t nodeId = 0) : mpMot(pMot), mNodeId(nodeId) {}
	public:
		// eval_xform(float frame)
		friend class GWMotion;
	};
protected:
	NodeInfo* mpNodeInfo;
	TrackInfo* mpTrackInfo;

public:
	GWMotion() = default;

	// load data create node groups and tracks
	bool load(const std::string& filePath);
	bool reset();

	void get_val(int trackId, int fno, GWVectorF& res);
	void eval(int trackId, float frame, GWVectorF& res);
	void eval_quat(int trackId, float frame, GWVectorF& res);

	// get node
	// get track
};

class MotGrpFunc {

};
