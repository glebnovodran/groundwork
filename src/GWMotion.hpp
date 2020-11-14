/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */
#include <cstring>
#include <map>

class GWMotion {
public:
	static const uint32_t NONE = (uint32_t)-1;

	struct TrackInfo {
		float* mpFrmData;
		GWVectorF mMinVal;
		GWVectorF mMaxVal;
		uint32_t mNumFrames;
		GWTrackKind mKind;
		uint8_t mDataMask;
		uint8_t mSrcMask;

		TrackInfo() : mpFrmData(nullptr), mMinVal(0), mMaxVal(0), mNumFrames(0),
			mKind(GWTrackKind::ROT), mDataMask(0), mSrcMask(0) {}
		~TrackInfo() { reset(); }

		void reset() {
			if (mpFrmData != nullptr) { delete[] mpFrmData; }
			mpFrmData = nullptr;
			mMinVal.fill(0.0f);
			mMaxVal.fill(0.0f);
			mNumFrames = 0;
			mKind = GWTrackKind::ROT;
			mDataMask = 0;
			mSrcMask = 0;
		}

		uint32_t get_stride() const {
			uint32_t val = 0;
			for (int i = 0; i < 3; ++i) {
				if (mDataMask & (1 << i)) { ++val; }
			}
			return val;
		}

		float* get_at(int32_t fno) const { return mpFrmData + get_stride() * fno; }

		GWVectorF get_vec_at(int32_t fno) const {
			GWVectorF val(0.0f);
			if (mKind == GWTrackKind::SCL) {
				val.fill(1.0f);
			}
			float* pData = get_at(fno);
			for (int i = 0; i < 3; ++i) {
				if (mDataMask & (1 << i)) {
					val[i] = *pData++;
				} else if (mSrcMask & (1 << i)) {
					val[i] = mMinVal[i];
				}
			}
			return val;
		};

		void create_from_raw(GWVectorF* pRawData, uint32_t len, uint8_t srcMask);
		void replace_data(GWVectorF* pRawData);
	};

	class Track {
	private:
		const GWMotion* mpMot;
		uint32_t mNodeId;
		GWTrackKind mKind;

		Track(const GWMotion* pMot, uint32_t nodeId = 0, GWTrackKind trackKind = GWTrackKind::ROT) : mpMot(pMot), mNodeId(nodeId), mKind(trackKind) {}
	public:
		GWVectorF eval(float frame) const {
			return is_valid() ? mpMot->eval(mNodeId, mKind, frame) : GWVectorF(0.0f);;
		}
		GWQuaternionF eval_quat(float frame, bool useSlerp = false) const {
			return is_valid() ? mpMot->eval_quat(mNodeId, frame, useSlerp) : GWQuaternionF::get_zero();
		}
		
		bool is_valid() const { return (mNodeId != NONE) && (mpMot != nullptr); }
		static const Track get_invalid() { return Track(nullptr, NONE); }

		uint32_t num_frames() const {
			return is_valid() ? mpMot->num_frames() : 0;
		}

		TrackInfo* get_track_info() const {
			return is_valid() ? mpMot->get_track_info(mNodeId, mKind) : nullptr;
		}

		GWTrackKind kind() const { return mKind; }

		uint32_t src_mask() const {
			uint32_t mask = 0;
			if (mpMot != nullptr) {
				const TrackInfo* pInfo = get_track_info();
				mask = pInfo->mSrcMask;
			}
			return mask;
		}
		uint32_t num_src_chan() const {
			uint32_t mask = src_mask();
			uint32_t num = 0;
			for (uint32_t i = 0; i < 3; ++i) {
				if (mask & (1 << i)) { ++num; }
			}
			return num;
		}

		const char* node_name() const {
			return mpMot == nullptr ? "" : mpMot->get_node_name(mNodeId);
		}

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
		char* pName;
		uint32_t numFrames;
		GWTransformOrder defXOrd;
		GWRotationOrder defROrd;

		NodeInfo() : pRotTrk(nullptr), pTrnTrk(nullptr), pSclTrk(nullptr), pXOrd(nullptr), pROrd(nullptr),
			pName(nullptr), numFrames(0), defXOrd(GWTransformOrder::SRT), defROrd(GWRotationOrder::XYZ) {}
		~NodeInfo() { reset(); }

		void reset() {
			pRotTrk = pTrnTrk = pSclTrk = nullptr;
			if (pXOrd != nullptr) { delete[] pXOrd; }
			if (pROrd != nullptr) { delete[] pROrd; }
			pName = nullptr;
			numFrames = 0;
			defXOrd = GWTransformOrder::SRT;
			defROrd = GWRotationOrder::XYZ;
		}

		GWTransformOrder get_xord(uint32_t frameNo) const {
			return (pXOrd == nullptr) ? defXOrd : pXOrd[frameNo % numFrames];
		}
		GWRotationOrder get_rord(uint32_t frameNo) const {
			return (pROrd == nullptr) ? defROrd : pROrd[frameNo % numFrames];
		}
		bool has_track(GWTrackKind kind) const { return pTrk[(uint8_t)kind] != nullptr; }
		TrackInfo* get_track_info(GWTrackKind kind) const { return pTrk[(uint8_t)kind]; }
	};

	class Node {
	private:
		const GWMotion* mpMot;
		uint32_t mNodeId;

		Node(const GWMotion* pMot, uint32_t nodeId = NONE) : mpMot(pMot), mNodeId(nodeId) {}
	public:
		const NodeInfo* get_node_info() const {
			return is_valid() ? mpMot->get_node_info(mNodeId) : nullptr;
		}

		const char* name() const {
			return mpMot == nullptr ? "" : mpMot->get_node_name(mNodeId);
		}

		Track get_track(GWTrackKind kind) {
			const NodeInfo* pNodeInfo = get_node_info();
			return pNodeInfo && pNodeInfo->has_track(kind) ? Track(mpMot, mNodeId, kind) : Track::get_invalid();
		}

		GWVectorF eval(GWTrackKind trackKind, float frame) const {
			return mpMot->eval(mNodeId, trackKind, frame);
		}
		GWQuaternionF eval_rot(float frame, bool useSlerp = false) const { return mpMot->eval_quat(mNodeId, frame, useSlerp); }
		GWVectorF eval_trn(float frame) const { return eval(GWTrackKind::TRN, frame); }
		GWVectorF eval_scl(float frame) const { return eval(GWTrackKind::SCL, frame); }

		void eval_xform(GWTransformF& xform, float frame) const {
			mpMot->eval_xform(xform, mNodeId, frame);
		}

		bool is_valid() const { return (mNodeId != NONE) && (mpMot != nullptr); }
		static const Node get_invalid() { return Node(nullptr, NONE); }
		friend class GWMotion;
	};

	enum RotDumpKind {
		DEG = 0,
		QUAT = 1,
		LOG = 2
	};
protected:
	std::map<const char*, uint32_t, bool(*)(const char*, const char*)> mNodeMap;
	NodeInfo* mpNodeInfo;
	TrackInfo* mpTrackInfo;
	char* mpStrData;
	void* mpExtMem;

	uint32_t mNumNodes;
	uint32_t mNumTracks;
	uint32_t mStrDataSz;

public:
	GWMotion() : mNodeMap([](const char* a, const char* b) { return ::strcmp(a, b) < 0; }),
		mpNodeInfo(nullptr), mpTrackInfo(nullptr), mpStrData(nullptr),
		mpExtMem(nullptr), mNumNodes(0), mNumTracks(0), mStrDataSz(0) {}

	bool load(const std::string& filePath);
	void unload();
	void clone_from(const GWMotion& mot);

	void alloc_binding_memory(uint32_t size);
	void release_binding_memory();
	void set_binding_memory(void* pMem) { mpExtMem = pMem; }
	bool binding_memory_allocated() const { return mpExtMem != nullptr; }
	template<typename T> T* get_binding_memory() { return reinterpret_cast<T*>(mpExtMem); }

	uint32_t find_node_id(const char* name) const {
		char* pName = const_cast<char*>(name);
		const auto it = mNodeMap.find(name);
		GWMotion::Node node(nullptr);
		return it == mNodeMap.cend() ? NONE : it->second;
	}
	Node get_node(const char* name) const {
		uint32_t id = find_node_id(name);
		return id == NONE ? Node::get_invalid() : Node(this, id);
	}
	Node get_node_by_id(uint32_t id) const {
		return id < mNumNodes ? Node(this, id) : Node::get_invalid();
	}

	const NodeInfo* get_node_info(uint32_t id) const { return id < mNumNodes ? &mpNodeInfo[id] : nullptr; }
	TrackInfo* get_track_info(uint32_t trackId) const { return trackId < mNumTracks ? &mpTrackInfo[trackId] : nullptr; }
	TrackInfo* get_track_info(uint32_t nodeId, GWTrackKind kind) const {
		const NodeInfo* pNodeInfo = get_node_info(nodeId);
		TrackInfo* pInfo = pNodeInfo == nullptr ? nullptr : pNodeInfo->get_track_info(kind);
		return pInfo;
	}

	const char* get_node_name(uint32_t nodeId) const {
		const NodeInfo* pInfo = get_node_info(nodeId);
		return pInfo == nullptr ? "" : pInfo->pName;
	}

	uint32_t num_nodes() const { return mNumNodes; }

	// all tracks have the same length
	uint32_t num_frames() const {
		return mpTrackInfo == nullptr ? 0 : mpTrackInfo[0].mNumFrames;
	}

	GWVectorF eval(uint32_t nodeId, GWTrackKind trackKind, float frame) const;
	GWQuaternionF eval_quat(uint32_t nodeId, float frame, bool useSlerp = false) const {
		GWQuaternionF res;

		if (useSlerp) {
			float fstart = ::floorf(frame);
			float bias = frame - fstart;
			if (0.0f == bias) {
				res = GWQuaternion::expmap_decode(eval(nodeId, GWTrackKind::ROT, fstart));
			} else {
				GWQuaternionF startVal = GWQuaternion::expmap_decode(eval(nodeId, GWTrackKind::ROT, fstart));
				GWQuaternionF endVal = GWQuaternion::expmap_decode(eval(nodeId, GWTrackKind::ROT, fstart+1.0f));
				res = GWUnitQuaternion::slerp(startVal, endVal, bias);
				res.normalize();
			}
		} else {
			res = GWQuaternion::expmap_decode(eval(nodeId, GWTrackKind::ROT, frame));
		}

		return res;
	}

	GWTransformOrder eval_xord(uint32_t nodeId, float frame) const;

	void eval_xform(GWTransformF& xform, uint32_t nodeId, float frame) const {
		GWQuaternionF qrot = eval_quat(nodeId, frame);
		GWVectorF trn = eval(nodeId, GWTrackKind::TRN, frame);
		GWVectorF scl = eval(nodeId, GWTrackKind::SCL, frame);
		GWTransformOrder xord = eval_xord(nodeId, frame);
		xform.make_transform(qrot, trn, scl, xord);
	}

	bool dump_clip(std::ostream& os, RotDumpKind rotDumpKind = RotDumpKind::QUAT, bool rle = false) const;
	void save_clip(const std::string& path, RotDumpKind rotDumpKind = RotDumpKind::QUAT, bool rle = false) const;

	friend std::ostream& operator << (std::ostream& os, GWMotion& mot) {
		mot.dump_clip(os);
		return os;
	}
};
