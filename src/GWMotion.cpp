/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */
#include <iostream>
#include <fstream>
#include <assert.h>
#include <TDMotion.hpp>
#include "groundwork.hpp"

static const bool HAS_NAMES = true;
static const bool COLUMN_CHANS = false;

void place_data(float* pDst, const GWVectorF* pRawData, uint32_t len, uint8_t dataMask) {
	if (pDst != nullptr) {
		float* pData = pDst;
		for (uint32_t fno = 0; fno < len; ++fno) {
			for (int i = 0; i < 3; ++i) {
				if (dataMask & (1 << i)) {
					*pData++ = pRawData[fno][i];
				}
			}
		}
	}
}

uint8_t calc_data_mask(const GWVectorF& minVal, const GWVectorF& maxVal) {
	uint8_t dataMask = 0;
	GWVectorF dim = maxVal - minVal;
	for (int i = 0; i < 3; ++i) {
		if (dim[i] != 0.0f) {
			dataMask |= (1 << i);
		}
	}
	return dataMask;
}

void GWMotion::TrackInfo::create_from_raw(GWVectorF* pRawData, uint32_t len, uint8_t srcMask) {
	assert(pFrmData == nullptr);
	this->srcMask = srcMask;
	uint32_t numChan = 0;

	GWTuple::calc_bbox(pRawData, len, minVal, maxVal);
	dataMask = calc_data_mask(minVal, maxVal);
	numChan = get_stride();
	pFrmData = new float[numChan * len];
	place_data(pFrmData, pRawData, len, dataMask);
	numFrames = len;
}

void GWMotion::TrackInfo::replace_data(GWVectorF* pRawData) {
	assert(pFrmData != nullptr);
	uint32_t numChan = 0;
	uint32_t oldNumChan = get_stride();

	GWTuple::calc_bbox(pRawData, numFrames, minVal, maxVal);
	dataMask = calc_data_mask(minVal, maxVal);
	numChan = get_stride();
	if (numChan != oldNumChan) {
		delete[] pFrmData;
		pFrmData = nullptr;
		uint32_t newSz = numChan * numFrames;
		if (newSz > 0) {
			pFrmData = new float[newSz];
		}
	}
	place_data(pFrmData, pRawData, numFrames, dataMask);
}


class CollectGrpFunc : public TDMotion::XformGrpFunc {
protected:
	const GWMotion* mpMot;
	const TDMotion* mpTDMot;
	uint32_t mNumTracks;
public:
	std::map<std::string, TDMotion::XformGrp> mGrpMap;

	CollectGrpFunc(const GWMotion* pMot, const TDMotion* pTDMot) {
		mpMot = pMot;
		mNumTracks = 0;
	}

	virtual void operator ()(std::string& name, TDMotion::XformGrp& grpInfo) {
		using namespace std;
		mGrpMap[name] = grpInfo;
		mNumTracks += calc_num_tracks(grpInfo);
	}

	size_t get_num_groups() const {
		return mGrpMap.size();
	}
	size_t get_num_tracks() const { return mNumTracks; }

	uint32_t calc_num_tracks(TDMotion::XformGrp& grpInfo) const {
		uint32_t numTracks = 0;
		if (grpInfo.has_rotation()) { ++numTracks; }
		if (grpInfo.has_translation()) { ++numTracks; }
		if (grpInfo.has_scale()) { ++numTracks; }
		return numTracks;
	}
	uint32_t calc_str_size() const {
		uint32_t size = 0;
		for (const auto& pair: mGrpMap) {
			size += pair.first.length();
		}
		return size;
	}
};

uint8_t get_raw_track_data(const TDMotion& tdmot, const TDMotion::XformGrp& grp, GWTrackKind kind,
	GWVectorF pOut[], float defVal = 0.0f) {
	const TDMotion::Channel* tdchan[3];
	size_t motLen = tdmot.length();
	uint8_t srcMask = 0;

	const size_t* pChanIdx = &grp.idx[(uint8_t)kind * 3];
	for (uint32_t i = 0; i < 3; ++i) {
		if (pChanIdx[i] != TDMotion::NONE) {
			srcMask |= 1 << i;
			tdchan[i] = tdmot.get_channel(pChanIdx[i]);
		} else { tdchan[i] = nullptr; }
	}

	for (uint32_t i = 0; i < motLen; ++i) {
		GWTuple3f tuple;
		for (uint32_t j = 0; j < 3; ++j) {
			tuple[j] = tdchan[j] == nullptr ? 0.0f : tdchan[j]->get_val(i);
		}
		pOut[i].from_tuple(tuple);
	}

	return srcMask;
}

bool GWMotion::load(const std::string & filePath) {
	using namespace std;
	TDMotion tdmot;
	CollectGrpFunc grpFunc(this, &tdmot);

	if (tdmot.load(filePath, HAS_NAMES, COLUMN_CHANS)) {
		tdmot.find_xforms(grpFunc);

		uint32_t numNodes = grpFunc.get_num_groups();
		uint32_t numTracks = grpFunc.get_num_tracks();

		mpNodeInfo = new NodeInfo[numNodes];
		mpTrackInfo = new TrackInfo[numTracks];
		size_t motLen = tdmot.length();
		GWVectorF* pTmpVec = new GWVectorF[motLen];

		NodeInfo* pNodeInfo = mpNodeInfo;
		TrackInfo* pTrackInfo = mpTrackInfo;
		mStrDataSz = grpFunc.calc_str_size() + numNodes;
		mpStrData = new char[mStrDataSz];
		char* pChar = mpStrData;
		uint32_t idx = 0;
		for (const auto& entry : grpFunc.mGrpMap) {
			cout << entry.first << endl;
			TDMotion::XformGrp grp = (TDMotion::XformGrp)entry.second;
			pNodeInfo->numFrames = motLen;
			pNodeInfo->pName = pChar;

			uint32_t sz = entry.first.length();
			entry.first.copy(pChar, sz);
			pChar += sz;
			*pChar = '\x0';
			++pChar;

			if (grp.has_rord()) {
				const TDMotion::Channel* pROrdChan = tdmot.get_channel(entry.second.rOrd);
				size_t num = pROrdChan->length();
				pNodeInfo->pROrd = new GWRotationOrder[num];
				for (size_t i = 0; i < pROrdChan->values.size(); ++i) {
					float val = pROrdChan->values[i];
					pNodeInfo->pROrd[i] = GWBase::rord_from_float(val);
				}
			}

			if (grp.has_xord()) {
				const TDMotion::Channel* pXOrdChan = tdmot.get_channel(entry.second.xOrd);
				size_t num = pXOrdChan->length();
				pNodeInfo->pXOrd = new GWTransformOrder[num];
				for (size_t i = 0; i < pXOrdChan->values.size(); ++i) {
					float val = pXOrdChan->values[i];
					pNodeInfo->pXOrd[i] = GWBase::xord_from_float(val);
				}
			}

			if (grp.has_translation()) {
				TrackInfo* pTrack = pTrackInfo++;
				uint8_t srcMask = get_raw_track_data(tdmot, grp, GWTrackKind::TRN, pTmpVec, 0.0f);

				pTrack->create_from_raw(pTmpVec, motLen, srcMask);
				pNodeInfo->pTrnTrk = pTrack;
			}

			if (grp.has_scale()) {
				TrackInfo* pTrack = pTrackInfo++;
				uint8_t srcMask = get_raw_track_data(tdmot, grp, GWTrackKind::SCL, pTmpVec, 1.0f);

				pTrack->create_from_raw(pTmpVec, motLen, srcMask);
				pNodeInfo->pSclTrk = pTrack;
			}

			if (grp.has_rotation()) {
				TrackInfo* pTrack = pTrackInfo++;
				uint8_t srcMask = get_raw_track_data(tdmot, grp, GWTrackKind::ROT, pTmpVec, 0.0f);
				
				GWQuaternionF prevQ;
				for (uint32_t fno = 0; fno < motLen; ++fno) {
					GWQuaternionF q;
					q.set_degrees(pTmpVec[fno].x, pTmpVec[fno].y, pTmpVec[fno].z, pNodeInfo->get_rord(fno));
					q.normalize();
					if (fno > 0) {
						bool flipFlg = (q.dot(prevQ) < 0.0f);
						if (flipFlg) { q.neg(); }
					}
					prevQ = q;
					pTmpVec[fno] = GWUnitQuaternion::log(q);
				}

				pTrack->create_from_raw(pTmpVec, motLen, srcMask);
				pNodeInfo->pRotTrk = pTrack;
			}

			mNodeMap[pNodeInfo->pName] = idx;
			++pNodeInfo;
			++idx;
		}

		mNumNodes = numNodes;
		mNumTracks = numTracks;
		delete[] pTmpVec;
		return true;
	}
	return false;
}

void GWMotion::unload() {
	mNodeMap.clear();
	delete[] mpTrackInfo;
	mpTrackInfo = nullptr;
	delete[] mpNodeInfo;
	mpNodeInfo = nullptr;
	delete[] mpStrData;
	mpStrData = nullptr;
	mNumNodes = 0;
	mNumTracks = 0;
	mStrDataSz = 0;
}

void GWMotion::clone_from(const GWMotion& mot) {
	unload();

	mpTrackInfo = new TrackInfo[mot.mNumTracks];
	mNumTracks = mot.mNumTracks;
	mpNodeInfo = new NodeInfo[mot.mNumNodes];
	mNumNodes = mot.mNumNodes;
	mpStrData = new char[mot.mStrDataSz];
	mStrDataSz = mot.mStrDataSz;
	::memcpy(mpStrData, mot.mpStrData, mot.mStrDataSz);

	for (uint32_t i = 0; i < mNumNodes; ++i) {
		mpNodeInfo[i] = mot.mpNodeInfo[i];
		for (uint32_t j = 0; j < 3; ++j) {
			const TrackInfo* pTrkInfo = mot.mpNodeInfo[i].pTrk[j];
			if (pTrkInfo == nullptr) {
				mpNodeInfo[i].pTrk[j] = nullptr;
			} else {
				mpNodeInfo[i].pTrk[j] = mpTrackInfo + (pTrkInfo - mot.mpTrackInfo);
				*mpNodeInfo[i].pTrk[j] = *pTrkInfo;

				uint32_t numFrames = pTrkInfo->numFrames;
				float* pFrameData = new float[numFrames];
				::memcpy(pFrameData, pTrkInfo->pFrmData, numFrames * sizeof(float));
				mpNodeInfo[i].pTrk[j]->pFrmData = pFrameData;
			}
		}
		uint32_t offs = mot.mpNodeInfo[i].pName - mot.mpStrData;
		mpNodeInfo[i].pName = mot.mpStrData + offs;
	}
	for (uint32_t i = 0; i < mNumNodes; ++i) {
		mNodeMap[mpNodeInfo[i].pName] = i;
	}
}

GWTransformOrder GWMotion::eval_xord(uint32_t nodeId, float frame) const {
	const NodeInfo* pInfo = get_node_info(nodeId);
	if (pInfo == nullptr) { return GWTransformOrder::SRT; }
	int fno = ::floorf(frame);
	int maxFrame = pInfo->numFrames - 1;
	fno = fno % int32_t(pInfo->numFrames);
	fno += pInfo->numFrames;
	return pInfo->get_xord(fno);
}

GWVectorF GWMotion::eval(uint32_t nodeId, GWTrackKind trackKind, float frame) const {
	GWVectorF val(0.0f);
	if (trackKind == GWTrackKind::SCL) {
		val.fill(1.0f);
	}
	if (nodeId < mNumNodes) {
		const TrackInfo* pTrack = mpNodeInfo[nodeId].get_track_info(trackKind);
		if (pTrack != nullptr) {
			float len = (float)pTrack->numFrames;
			float maxFrame = len - 1.0f;
			float f = ::fmodf(frame, len);
			f = f < 0 ? f + len : f;
			float fstart = ::truncf(f);
			float bias = f - fstart;
			int32_t istart = (int32_t)fstart;
			val = pTrack->get_vec_at(istart);
			if (bias > 0.0f) {
				int32_t iend = (istart == maxFrame) ? 0 : istart + 1;
				GWVectorF valB = pTrack->get_vec_at(iend);
				GWTuple::lerp(val, valB, bias);
			}
		}
	}
	return val;
}

void dump_track_to_clip(std::ostream & os, const GWMotion::Track& track) {
	using namespace std;

	uint32_t numFrames = track.num_frames();
	const GWMotion::TrackInfo* pInfo = track.get_track_info();
	uint32_t srcMask = pInfo->srcMask;
	const char* nodeName = track.node_name();

	for (uint32_t i = 0; i < 3; ++i) {
		if (srcMask & (1 << i)) {
			os << "   {" << endl;
			os << "      name = " << nodeName << ":" << "rts"[(uint32_t)track.kind()] << "xyz"[i] << endl;
			os << "      data = ";
			for (uint32_t fno = 0; fno < numFrames; ++fno) {
				float val = track.eval(float(fno))[i];
				os << " " << val;
			}
			os << endl;
			os << "   }" << endl;
		}
	}
}

void dump_rot_track_to_clip(std::ostream & os, const GWMotion::Track& track, GWMotion::RotDumpKind dumpKind) {
	using namespace std;

	uint32_t numFrames = track.num_frames();
	const GWMotion::TrackInfo* pInfo = track.get_track_info();
	uint32_t mask = pInfo->srcMask;
	const char* nodeName = track.node_name();
	uint32_t maxComp = (dumpKind == GWMotion::RotDumpKind::QUAT) ? 4 : 3;
	
	char prefix;
	if ((dumpKind == GWMotion::RotDumpKind::QUAT) || (dumpKind == GWMotion::RotDumpKind::LOG)) {
		mask = (1 << maxComp) - 1; // dump all components
		prefix = 'q';
	} else { prefix = 'r'; }

	for (uint32_t i = 0; i < maxComp; ++i) {
		if (mask & (1 << i)) {
			os << "   {" << endl;
			os << "      name = " << nodeName << ":" << prefix << "xyzw"[i] << endl;
			os << "      data = ";
			for (uint32_t fno = 0; fno < numFrames; ++fno) {
				GWVectorF vec = track.eval(float(fno));
				float val = 0.0f;
				switch (dumpKind) {
				case GWMotion::RotDumpKind::DEG: {
						GWQuaternionF q = GWQuaternion::expmap_decode(vec);
						vec = GWUnitQuaternion::get_degrees(q);
						val = vec[i];
					}
					break;
				case GWMotion::RotDumpKind::QUAT: {
						GWQuaternionF q = GWQuaternion::expmap_decode(vec);
						val = q.get_tuple()[i];
					}
					break;
				case GWMotion::RotDumpKind::LOG:
					val = vec[i];
					break;
				}
				os << " " << val;
			}
			os << endl;
			os << "   }" << endl;
		}
	}
}

bool GWMotion::dump_clip(std::ostream & os, RotDumpKind rotDumpKind, bool rle) const {
	using namespace std;
	// TODO: support for rord, xord
	if (!os.good()) { return false; }
	uint32_t numTracks = 0;
	for (uint32_t id = 0; id < mNumNodes; ++id) {
		Node node = get_node_by_id(id);
		Track track = node.get_track(GWTrackKind::ROT);
		if (track.is_valid()) {
			switch (rotDumpKind) {
				case GWMotion::RotDumpKind::DEG:
					numTracks += track.num_src_chan();
					break;
				case GWMotion::RotDumpKind::QUAT:
					numTracks += 4;
					break;
				case GWMotion::RotDumpKind::LOG:
					numTracks += 3;
					break;
			}
		}
		track = node.get_track(GWTrackKind::TRN);
		if (track.is_valid()) { numTracks += track.num_src_chan(); }
		track = node.get_track(GWTrackKind::SCL);
		if (track.is_valid()) { numTracks += track.num_src_chan(); }
	}

	os << "{" << endl;
	os << "\trate = 60" << endl;
	os << "\tstart = -1" << endl;
	os << "\ttracklength = " << mpTrackInfo[0].numFrames << endl;
	os << "\ttracks = " << numTracks << endl;

	for (uint32_t id = 0; id < mNumNodes; ++id) {
		Node node = get_node_by_id(id);
		const char* nodeName = node.name();
		Track track = node.get_track(GWTrackKind::ROT);
		if (track.is_valid()) {
			dump_rot_track_to_clip(os, track, rotDumpKind);
		}
		track = node.get_track(GWTrackKind::TRN);
		if (track.is_valid()) {
			dump_track_to_clip(os, track);
		}
		track = node.get_track(GWTrackKind::SCL);
		if (track.is_valid()) {
			dump_track_to_clip(os, track);
		}
	}
	os << "}" << endl;
	return true;
}

void GWMotion::save_clip(const std::string & path, RotDumpKind rotDumpKind, bool rle) const {
	using namespace std;
	ofstream os(path);
	dump_clip(os, rotDumpKind, rle);
	os.close();
}
