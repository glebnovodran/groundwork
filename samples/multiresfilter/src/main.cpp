/*
 * Motion multiresolution filtering sample program
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <iostream>
#include <groundwork.hpp>
#include "filter.hpp"


int main(int argc, char* argv[]) {
	using namespace std;
	const string fname = "../../data/walk_rn.txt";
	GWMotion mot;

	if (mot.load(fname)) {
		GWVectorF v = mot.eval(0, GWTrackKind::ROT, 73.0f);
		GWMotion cloned;
		cloned.clone_from(mot);
		GWVectorF v1 = cloned.eval(0, GWTrackKind::ROT, 73.0f);
		MotionEqualizer equ(mot);
		uint32_t numGains = equ.get_num_bands();
		float* pGains = new float[numGains];
		for (int i = 0; i < numGains; ++i) { pGains[i] = 4.0f; }
		equ.build();
		float gains[7] = {1.0f, 1.0f, 1.73f, 2.7f, 1.73f, 1.0f, 1.0f};
		uint32_t nodeId = equ.get_motion()->find_node_id("/obj/ANIM/j_Knee_L");
		equ.apply(nodeId, nullptr, numGains, gains);
		equ.get_motion()->save_clip("original.clip", GWMotion::RotDumpKind::DEG);
		equ.get_equalized()->save_clip("equalized.clip", GWMotion::RotDumpKind::DEG);
		equ.reset();
		mot.unload();
		return 0;
	}

	return -1;
}
