/*
 * Groundwork transform operations tests
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <groundwork.hpp>
#include "test.hpp"

static bool test_xform_invert() {
	GWTransformF xform1 = {
		1, 0, 0, 0,
		0, 2, 0, 0,
		0, 0, 3, 0,
		3, 2, 1, 1
	};
	GWVectorF c(1.0f, 0.0f, 0.0f);
	GWVectorF a = xform1.calc_pnt(c);
	GWTransformF invXfrom = xform1.get_inverted_fast();
	GWVectorF b = invXfrom.calc_pnt(a);
	if (!GWTuple::compare(c, b, 0.001f)) {
		return false;
	}

	invXfrom = xform1.get_inverted();
	b = invXfrom.calc_pnt(a);
	if (!GWTuple::compare(c, b, 0.001f)) {
		return false;
	}

	GWTransformF xformSRT;
	GWQuaternionF qrot;
	qrot.set_degrees(15.0f, 30.0, 45.0f, GWRotationOrder::ZXY);
	xformSRT.make_transform(qrot, GWVectorF(1, 2, 3), GWVectorF(4, 2, 1));
	GWTransformF invSRT = xformSRT.get_inverted();
	a = xformSRT.calc_pnt(c);
	b = invSRT.calc_pnt(a);
	if (!GWTuple::compare(c, b, 0.001f)) {
		return false;
	}

	GWTransformF invSRT1 = xformSRT.get_inverted_fast();
	a = xformSRT.calc_pnt(c);
	GWVectorF b0 = invSRT1.calc_pnt(a);
	if (!GWTuple::compare(c, b0, 0.001f)) {
		return false;
	}
	return true;
}

static TEST_ENTRY s_xform_tests[] = {
	TEST_DECL(test_xform_invert),
};

bool test_xform() {
	return 0 == EXEC_TESTS(s_xform_tests);
}