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

		equ.build();
		mot.unload();
		return 0;
	}

	return -1;
}
