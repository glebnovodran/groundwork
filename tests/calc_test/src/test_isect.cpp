/*
 * Groundwork intersection routines tests
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <iostream>
#include <groundwork.hpp>
#include "test.hpp"

struct SegTriTest {
	struct TestRes {
		GWVectorF hitPos;
		GWVectorF hitNrm;
		int triId;
	};

	int mNseg;
	int mNtri;
	GWVectorF* mpSegPts;
	GWVectorF* mpTriPts;
	TestRes* mpRes;

	void read_vec(std::ifstream& ifs, GWVectorF* pVec) {
		for (int j=0; j < 3; ++j) {
			ifs >> (*pVec)[j];
		}
	}
	bool load_data(const char* fpath) {
		using namespace std;
		ifstream ifs(fpath);

		if (ifs.good()) {
			ifs >>  mNseg;
			ifs >>  mNtri;
			if ((mNseg < 0) || (mNtri < 0)) {return false;}
			mpSegPts = new GWVectorF[mNseg*2];
			mpTriPts = new GWVectorF[mNtri*3];
			mpRes = new TestRes[mNseg]; // the test data is organized the way it can contain only a single intersection
			for (int i = 0; i < mNseg; ++i) {
				read_vec(ifs, &mpSegPts[i*2]);
				read_vec(ifs, &mpSegPts[i*2+1]);
			}
			for (int i = 0; i < mNtri; ++i) {
				read_vec(ifs, &mpTriPts[i*3]);
				read_vec(ifs, &mpTriPts[i*3+1]);
				read_vec(ifs, &mpTriPts[i*3+2]);
			}
			for (int i = 0; i < mNseg; ++i) {
				ifs >> mpRes[i].triId;
				read_vec(ifs, &mpRes[i].hitPos);
				read_vec(ifs, &mpRes[i].hitNrm);
			}
		}
		ifs.close();
		return true;
	}

	bool run() {
		bool hitRes;
		GWVectorF hitPos, hitNrm;
		for(int i = 0; i < mNseg; i++) {
			int isectTriId = mpRes[i].triId;
			for(int j = 0; j < mNtri; j++) {
				hitRes = GWIntersect::seg_tri_cw(mpSegPts[2*i], mpSegPts[2*i+1],
					mpTriPts[3*j], mpTriPts[3*j+1], mpTriPts[3*j+2], &hitPos, &hitNrm);
				if (hitRes) {
					if (isectTriId != j) {
						GWSys::dbg_msg("An unexpected intersection result for segment %d, triangle %d", i, j);
						return false;
					}
					if (!GWTuple::almost_equal(hitPos, mpRes[i].hitPos, 0.001f)) {
						GWSys::dbg_msg("Hit position comparision failed for segment %d, triangle %d", i, j);
						return false;
					}
					if (!GWTuple::almost_equal(hitPos, mpRes[i].hitPos, 0.001f)) {
						GWSys::dbg_msg("Hit position comparision failed for segment %d, triangle %d", i, j);
						return false;
					}
				} else if (isectTriId == j) {
					GWSys::dbg_msg("False negative for segment %d, triangle %d", i, j);
					return false;
				}
			}
		}
		return true;
	}
	void reset() {
		delete[] mpSegPts;
		delete[] mpTriPts;
		delete[] mpRes;
	}

};

static bool test_isect_seg_tri() {
	using namespace std;
	SegTriTest test;
	const char* path = GWApp::get_full_path();
	bool loaded = test.load_data("./data/calc_test/seg_tri.dat");
	if (!loaded) {
		GWSys::dbg_msg("Failed to load seg/tri intersection data");
		return false;
	}
	bool res0 = test.run();
	if (!res0) {
		GWSys::dbg_msg("Failed 'preset' seg-tri intersection test");
	}
	test.reset();
	loaded = test.load_data("./data/calc_test/seg_tri_scatter.dat");
	if (!loaded) {
		GWSys::dbg_msg("Failed to load seg/tri intersection data");
		return false;
	}
	bool res1 = test.run();
	if (!res1) {
		GWSys::dbg_msg("Failed 'scattered' seg-tri intersection test");
	}
	return res0 & res1;
}

static TEST_ENTRY s_isect_tests[] = {
	TEST_DECL(test_isect_seg_tri),
};

bool test_isect() {
	return 0 == EXEC_TESTS(s_isect_tests);
}