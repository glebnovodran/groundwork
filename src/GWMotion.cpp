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

void GWMotion::TrackInfo::create_from_raw(GWVectorF* pRawData, uint32_t len, uint8_t srcMask) {
	assert(pFrmData == nullptr);
	GWVectorF dim = maxVal - minVal;
	int numChan = 0;
	for (int i = 0; i < 3; ++i) {
		if (dim[i] != 0.0f) {
			dataMask |= (1 << i);
			++numChan;
		}
	}
	pFrmData = new float[numChan * len];
	float* pData = pFrmData;
	for (uint32_t fno = 0; fno < len; ++fno) {
		for (int i = 0; i < 3; ++i) {
			if (dataMask & (1 << i)) {
				*pData++ = pRawData[fno][i];
			}
		}
	}
	numFrames = len;
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
	GWVectorF pOut[], GWVectorF& minVal, GWVectorF& maxVal, float defVal = 0.0f) {
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
		if (i == 0) {
			minVal = pOut[i];
			maxVal = pOut[i];
		} else {
			GWTuple::max(maxVal, pOut[i], maxVal);
			GWTuple::min(minVal, pOut[i], minVal);
		}
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
				uint8_t srcMask = get_raw_track_data(tdmot, grp, GWTrackKind::TRN,
					pTmpVec, pTrack->minVal, pTrack->maxVal, 0.0f);

				pTrack->create_from_raw(pTmpVec, motLen, srcMask);
				pTrack->srcMask = srcMask;
				pNodeInfo->pTrnTrk = pTrack;
			}

			if (grp.has_scale()) {
				TrackInfo* pTrack = pTrackInfo++;
				uint8_t srcMask = get_raw_track_data(tdmot, grp, GWTrackKind::SCL,
					pTmpVec, pTrack->minVal, pTrack->maxVal, 1.0f);

				pTrack->create_from_raw(pTmpVec, motLen, srcMask);
				pTrack->srcMask = srcMask;
				pNodeInfo->pSclTrk = pTrack;
			}

			if (grp.has_rotation()) {
				TrackInfo* pTrack = pTrackInfo++;
				uint8_t srcMask = get_raw_track_data(tdmot, grp, GWTrackKind::ROT,
					pTmpVec, pTrack->minVal, pTrack->maxVal, 0.0f);
				
				GWQuaternionF firstQ;
				for (uint32_t fno = 0; fno < motLen; ++fno) {
					GWQuaternionF q;
					q.set_degrees(pTmpVec[fno].x, pTmpVec[fno].y, pTmpVec[fno].z, pNodeInfo->get_rord(fno));
					q.normalize();
					if (fno == 0) {
						firstQ = q;
					} else {
						bool flipFlg = (q.dot(firstQ) < 0.0f);
						if (flipFlg) { q.neg(); }
					}
					pTmpVec[fno] = GWUnitQuaternion::log(q);

					if (fno == 0) {
						pTrack->minVal = pTmpVec[0];
						pTrack->maxVal = pTmpVec[0];
					} else {
						GWTuple::max(pTrack->maxVal, pTmpVec[fno], pTrack->maxVal);
						GWTuple::min(pTrack->minVal, pTmpVec[fno], pTrack->minVal);
					}
				}

				pTrack->create_from_raw(pTmpVec, motLen, srcMask);
				pTrack->srcMask = srcMask;
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
	TrackInfo* pTrackInfo = mpTrackInfo;
	for (uint32_t i = 0; i < mNumTracks; ++i) {
		delete[] pTrackInfo->pFrmData;
		++pTrackInfo;
	}
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
