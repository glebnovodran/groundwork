/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */
#include <iostream>
#include <map>
#include <assert.h>
#include <TDMotion.hpp>
#include "groundwork.hpp"
//#include "GWMotion.hpp"

static bool HAS_NAMES = true;
static bool COLUMN_CHANS = false;

class CollectGrpFunc : public TDMotion::XformGrpFunc {
protected:
	const GWMotion* mpMot;
	const TDMotion* mpTDMot; // TODO: remove ?
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
};
/*
GWMotion::TransformNode* GWMotion::add_node(const std::string& name) {
	TransformNode* pNode = mNodeMap[name];
	if (pNode == nullptr) {
		mNodes.emplace_back();
		pNode = &mNodes.back();
		pNode->pMot = this;
		mNodeMap[name] = pNode;
	}
	return pNode;
}
*/

uint8_t get_raw_track_data(const TDMotion& tdmot, const TDMotion::XformGrp& grp, GWTrackKind kind,
	GWVectorF pOut[], GWVectorF& minVal, GWVectorF& maxVal, float defVal = 0.0f) {
	const TDMotion::Channel* tdchan[3];
	size_t motLen = tdmot.length();
	uint8_t srcMask = 0;
	minVal.fill(FLT_MAX);
	maxVal.fill(-FLT_MAX);

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
		GWTuple::max(maxVal, pOut[i], maxVal);
		GWTuple::min(minVal, pOut[i], minVal);
	}

	return srcMask;
}

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
	for (int fno = 0; fno < len; ++fno) {
		for (int i = 0; i < 3; ++i) {
			if (dataMask & (1 << i)) {
				*pData++ = pRawData[fno][i];
			}
		}
	}

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

		NodeInfo* pNodeInfo = const_cast<NodeInfo*>(mpNodeInfo);
		TrackInfo* pTrackInfo = const_cast<TrackInfo*>(mpTrackInfo); // !

		for (const auto& entry : grpFunc.mGrpMap) {
			cout << entry.first << endl;
			TDMotion::XformGrp grp = (TDMotion::XformGrp)entry.second;
			if (grp.has_rord()) {
				const TDMotion::Channel* pROrdChan = tdmot.get_channel(entry.second.rOrd);
				size_t num = pROrdChan->length();
				pNodeInfo->pROrd = new GWRotationOrder[num];
				pNodeInfo->rordLen = num;
				for (size_t i = 0; i < pROrdChan->values.size(); ++i) {
					float val = pROrdChan->values[i];
					pNodeInfo->pROrd[i] = (GWRotationOrder)(uint8_t)val;
				}
			}

			if (grp.has_xord()) {
				const TDMotion::Channel* pXOrdChan = tdmot.get_channel(entry.second.xOrd);
				size_t num = pXOrdChan->length();
				pNodeInfo->pXOrd = new GWTransformOrder[num];
				pNodeInfo->xordLen = num;
				for (size_t i = 0; i < pXOrdChan->values.size(); ++i) {
					float val = pXOrdChan->values[i];
					pNodeInfo->pXOrd[i] = (GWTransformOrder)(uint8_t)val;
				}
			}

			if (grp.has_translation()) {
				TrackInfo* pTrack = pTrackInfo++;
				uint8_t srcMask = get_raw_track_data(tdmot, grp, GWTrackKind::TRN,
					pTmpVec, pTrack->minVal, pTrack->maxVal, 0.0f);

				pTrack->create_from_raw(pTmpVec, motLen, srcMask);
				pTrack->srcMask = srcMask;
			}

			if (grp.has_scale()) {
				TrackInfo* pTrack = pTrackInfo++;
				uint8_t srcMask = get_raw_track_data(tdmot, grp, GWTrackKind::SCL,
					pTmpVec, pTrack->minVal, pTrack->maxVal, 1.0f);

				pTrack->create_from_raw(pTmpVec, motLen, srcMask);
				pTrack->srcMask = srcMask;
			}

			if (grp.has_rotation()) {
				TrackInfo* pTrack = pTrackInfo++;
				uint8_t srcMask = get_raw_track_data(tdmot, grp, GWTrackKind::ROT,
					pTmpVec, pTrack->minVal, pTrack->maxVal, 0.0f);

				for (uint32_t fno = 0; fno < motLen; ++fno) {
					GWQuaternionF q;
					q.set_radians(pTmpVec->x, pTmpVec->y, pTmpVec->z, pNodeInfo->get_rord(fno));
					q.normalize();
					pTmpVec[fno] = GWUnitQuaternion::log(q);
					GWTuple::max(pTrack->maxVal, pTmpVec[fno], pTrack->maxVal);
					GWTuple::min(pTrack->minVal, pTmpVec[fno], pTrack->minVal);
				}

				pTrack->create_from_raw(pTmpVec, motLen, srcMask);
				pTrack->srcMask = srcMask;
			}

			++pNodeInfo;
		}

		delete[] pTmpVec;
		return true;
	}
	return false;
}

