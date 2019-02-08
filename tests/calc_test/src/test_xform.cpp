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

static bool test_3x4() {
	GWTransformF xform;
	GWTransform3x4F xform34;

	GWVectorF pnt(-1.0f, -4.0f, -8.0f);
	GWVectorF axisX(1.0f, 0.0f, 0.0f);

	xform.make_translation(2.0f, 4.0f, 8.0f);
	GWVectorF px = xform.calc_pnt(pnt);

	xform34 = GWXformCvt::get_3x4(xform);
	GWVectorF px34 = xform34.calc_pnt(pnt);
	if (!GWTuple::compare(px, px34, 0.001f)) {
		GWSys::dbg_msg("test_3x4: vector transform test failed");
		return false;
	}
	xform.make_rotation(15.0f, 30.0f, 45.0f);
	xform34 = GWXformCvt::get_3x4(xform);
	GWVectorF v = xform.calc_vec(axisX);
	GWVectorF v34 = xform34.calc_vec(axisX);
	if (!GWTuple::compare(v, v34, 0.001f)) {
		GWSys::dbg_msg("test_3x4: vector rotation test 0 failed");
		return false;
	}

	GWVectorF v0(1.0f, 1.0f, 0.0f);
	v0.normalize();
	v = xform.calc_vec(v0);
	v34 = xform34.calc_vec(v0);
	if (!GWTuple::compare(v, v34, 0.001f)) {
		GWSys::dbg_msg("test_3x4: vector rotation test 1 failed");
		return false;
	}

	xform.set_translation(1.0f, 2.0f, 3.0f);
	GWVectorF vv = xform.calc_vec(v0);
	if (!GWTuple::compare(v, vv, 0.001f)) {
		GWSys::dbg_msg("test_3x4: vector rotation test 1.1 failed");
		return false;
	}
	xform34.set_translation(1.0f, 2.0f, 3.0f);
	GWVectorF vv34 = xform.calc_vec(v0);
	if (!GWTuple::compare(v34, vv34, 0.001f)) {
		GWSys::dbg_msg("test_3x4: vector rotation test 1.2 failed");
		return false;
	}

	GWQuaternionF q;
	q.set_degrees(15.0f, 30.0f, 45.0f, GWRotationOrder::XYZ);
	GWTransform3x4F xq34;
	xq34.make_rotation(q);
	xform.make_rotation(15.0f, 30.0f, 45.0f, GWRotationOrder::XYZ);
	xform34 = GWXformCvt::get_3x4(xform);

	if (!compare_mtx(xq34.as_tptr(), xform34.as_tptr(), 3, 4)) {
		GWSys::dbg_msg("make_rotation failed");
		return false;
	}
	return true;
}

static TEST_ENTRY s_xform_tests[] = {
	TEST_DECL(test_xform_invert),
	TEST_DECL(test_3x4),
};

bool test_xform() {
	return 0 == EXEC_TESTS(s_xform_tests);
}