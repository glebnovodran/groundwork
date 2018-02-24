/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

class GWMotion {
public:
	static const uint32_t NONE = (uint32_t)-1;

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

		uint32_t get_stride() const {
			uint32_t val = 0;
			for (int i = 0; i < 3; ++i) {
				if (dataMask & (1 << i)) { ++val; }
			}
			return val;
		}

		float* get_at(int32_t fno) const { return pFrmData + get_stride() * fno; }

		GWVectorF get_vec_at(int32_t fno) const {
			GWVectorF val(0.0f);
			if (kind == GWTrackKind::SCL) {
				val.fill(1.0f);
			}
			float* pData = get_at(fno);
			for (int i = 0; i < 3; ++i) {
				if (dataMask & (1 << i)) {
					val[i] = *pData++;
				} else if (srcMask & (1 << i)) {
					val[i] = minVal[i];
				}
			}
			return val;
		};

		void create_from_raw(GWVectorF* pRawData, uint32_t len, uint8_t srcMask);
	};

	class Track {
	private:
		const GWMotion* mpMot;
		uint32_t mNodeId;
		GWTrackKind mKind;

		Track(const GWMotion* pMot, uint32_t nodeId = 0, GWTrackKind trackKind = GWTrackKind::ROT) : mpMot(pMot), mNodeId(nodeId), mKind(trackKind) {}
	public:
		GWVectorF eval(float frame) const {
			return mpMot->eval(mNodeId, mKind, frame);
		}
		GWQuaternionF eval_quat(float frame, bool useSlerp = false) const {
			return mpMot->eval_quat(mNodeId, frame, useSlerp);
		}
		
		bool is_invalid() const { (mNodeId == NONE) && (mpMot == nullptr); }
		static const Track invalid() { return Track(nullptr, NONE); }
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
		bool has_track(GWTrackKind kind) const { return pTrk[(uint8_t)kind] != nullptr; }
		const TrackInfo* get_track_info(GWTrackKind kind) const { return pTrk[(uint8_t)kind]; }
	};

	class Node {
	private:
		const GWMotion* mpMot;
		uint32_t mNodeId;

		Node(const GWMotion* pMot, uint32_t nodeId = 0) : mpMot(pMot), mNodeId(nodeId) {}
	public:
		// eval_xform(float frame)
		const NodeInfo* get_node_info() const {
			return mpMot->get_node_info(mNodeId);
		}
		Track get_track(GWTrackKind kind) {
			const NodeInfo* pNodeInfo = get_node_info();
			return pNodeInfo->has_track(kind) ? Track(mpMot, mNodeId, kind) : Track::invalid();
		}

		bool is_invalid() const { (mNodeId == NONE) && (mpMot == nullptr); }

		friend class GWMotion;
	};
protected:
	NodeInfo* mpNodeInfo;
	TrackInfo* mpTrackInfo;
	uint32_t mNumNodes;
	uint32_t mNumTracks;

public:
	GWMotion() = default;

	bool load(const std::string& filePath);
	void unload();

	Node get_node(const char* name) const;
	Node get_node_by_id(uint32_t id) const;

	NodeInfo* get_node_info(uint32_t id) const { return id < mNumNodes ? &mpNodeInfo[id] : nullptr; }
	TrackInfo* get_track_info(uint32_t id) const { return id < mNumTracks ? &mpTrackInfo[id] : nullptr; }

	GWVectorF eval(uint32_t nodeId, GWTrackKind trackKind, float frame) const;
	GWQuaternionF eval_quat(uint32_t nodeId, float frame, bool useSlerp = false) const {
		return GWQuaternion::expmap_decode(eval(nodeId, GWTrackKind::ROT, frame));
	}
	// eval_xform(uint32_t nodeId, float frame);

	bool dump_clip(std::ostream& os, bool quatRot = true, bool quatLog = false, bool rle = false) const;
	void save_clip(const std::string& path, bool rle = false) const;

	friend std::ostream& operator << (std::ostream& os, GWMotion& mot);
protected:
	GWVectorF get_vec_at(const TrackInfo* pTrack);
};
