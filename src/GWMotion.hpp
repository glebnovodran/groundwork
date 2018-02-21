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
		GWVectorF get_val(int fno); // ==> mpMot->get_val(mNodeId, mTrackKind, fno)
		GWVectorF eval(float frame);
		GWQuaternionF eval_quat(float frame);

		friend class GWMotion;
	};

	struct NodeInfo {
		union {
			struct {
				TrackInfo* pRotTrk;
				TrackInfo* pTrnTrk;
				TrackInfo* pSclTrk;
			};
			TrackInfo* pTrk[3];
		};
		GWTransformOrder* pXOrd;
		GWRotationOrder* pROrd;
		uint32_t numFrames;
		GWTransformOrder defXOrd;
		GWRotationOrder defROrd;

		NodeInfo() : pRotTrk(nullptr), pTrnTrk(nullptr), pSclTrk(nullptr), pXOrd(nullptr), pROrd(nullptr),
			numFrames(0), defXOrd(GWTransformOrder::RST), defROrd(GWRotationOrder::XYZ) {}

		GWTransformOrder get_xord(uint32_t frameNo) const {
			return (pXOrd == nullptr) ? GWTransformOrder::RST : pXOrd[frameNo % numFrames];
		}
		GWRotationOrder get_rord(uint32_t frameNo) const {
			return (pROrd == nullptr) ? GWRotationOrder::XYZ : pROrd[frameNo % numFrames];
		}

		const TrackInfo* get_track_info(GWTrackKind kind) const;
	};

	class Node {
	private:
		const GWMotion* mpMot;
		uint32_t mNodeId;

		Node(const GWMotion* pMot, uint32_t nodeId = 0) : mpMot(pMot), mNodeId(nodeId) {}
	public:
		// eval_xform(float frame)
		Track get_track(GWTrackKind kind);
		friend class GWMotion;
	};
protected:
	NodeInfo* mpNodeInfo;
	TrackInfo* mpTrackInfo;
	uint32_t mNumNodes;
	uint32_t mNumTracks;

public:
	GWMotion() = default;

	// load data create node groups and tracks
	bool load(const std::string& filePath);
	bool reset();

	Node get_node(const char* name) const;
	Node get_node_by_id(uint32_t id) const;

	GWVectorF get_val(uint32_t nodeId, GWTrackKind trackKind, int fno) const;
	GWVectorF eval(uint32_t nodeId, GWTrackKind trackKind, float frame) const;
	GWQuaternionF eval_quat(uint32_t nodeId, float frame, bool useSlerp = false) const {
		return GWQuaternion::expmap_decode(eval(nodeId, GWTrackKind::ROT, frame));
	}
	// eval_xform(uint32_t nodeId, float frame);

};
