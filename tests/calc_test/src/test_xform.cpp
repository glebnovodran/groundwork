/*
 * Groundwork transform operations tests
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <groundwork.hpp>
#include "test.hpp"

static bool test_make_rotation() {
	GWTransformF testData = {
		0.520866f, 0.703879f, -0.482963f, 0,
		-0.683013f, 0.683013f, 0.258819f, 0,
		0.512047f, 0.19506f, 0.836516f, 0,
		0, 0, 0, 1
	};

	GWTransformF xform;
	xform.make_deg_rotation(15.0f, 30.0f, 45.0f, GWRotationOrder::YXZ);
	bool res = GWMatrix::tup_almost_eq(xform.as_tptr(), testData.as_tptr(), 16, 0.001f);
	return res;
}

static bool test_make_xform() {
	GWTransformF testData = {
		1.83712f, 1.44183f, -0.324469f, 0,
		-2.12132f, 1.36603f, 0.183013f, 0,
		1.06066f, 0.234725f, 0.928023f, 0,
		1.83712f, 5.11278f, 3.75365f, 1
	};

	GWTransformF xform;
	GWQuaternionF q;
	q.set_degrees(15.0f, 30.0f, 45.0f, GWRotationOrder::YZX);
	GWVectorF trn(1.0f, 2.0f, 4.0f);
	GWVectorF scl(3.0f, 2.0f, 1.0f);
	xform.make_transform(q, trn, scl, GWTransformOrder::TRS);
	bool res = xform.compare(testData, 0.001f);

	GWTransform3x4F xform34;
	xform34.make_transform(q, trn, scl, GWTransformOrder::TRS);
	GWTransformF xform1 = GWXformCvt::get_4x4(xform34);
	res &= xform.compare(xform1, 0.001f);

	return res;
}

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

static bool test_3x4_apply() {
	GWTransformF xform;
	GWTransform3x4F xform34;
	GWVectorF pnt(-1.0f, -4.0f, -8.0f);

	xform.make_translation(2.0f, 4.0f, 8.0f);
	GWVectorF px = xform.calc_pnt(pnt);

	xform34 = GWXformCvt::get_3x4(xform);
	GWVectorF px34 = xform34.calc_pnt(pnt);
	if (!GWTuple::compare(px, px34, 0.001f)) {
		GWSys::dbg_msg("test_3x4: vector transform test failed");
		return false;
	}

	return true;
}

static bool test_3x4_rotation() {
	GWTransformF xform;
	GWTransform3x4F xform34;
	GWVectorF axisX(1.0f, 0.0f, 0.0f);

	xform.make_rotation(
		GWBase::radians(15.0f),
		GWBase::radians(30.0f),
		GWBase::radians(45.0f)
		);

	xform34 = GWXformCvt::get_3x4(xform);
	GWVectorF v = xform.calc_vec(axisX);
	GWVectorF v34 = xform34.calc_vec(axisX);
	if (!GWTuple::compare(v, v34, 0.001f)) {
		GWSys::dbg_msg("test_3x4: vector rotation test failed");
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
	xform.make_rotation(
		GWBase::radians(15.0f),
		GWBase::radians(30.0f),
		GWBase::radians(45.0f)
	);
	xform34 = GWXformCvt::get_3x4(xform);

	if (!compare_mtx(xq34.as_tptr(), xform34.as_tptr(), 3, 4)) {
		GWSys::dbg_msg("make_rotation failed");
		return false;
	}
	return true;
}

static TEST_ENTRY s_xform3x4_tests[] = {
	TEST_DECL(test_3x4_apply),
	TEST_DECL(test_3x4_rotation),
};

static bool test_3x4() {
	return 0 == EXEC_TESTS(s_xform3x4_tests);
}

static TEST_ENTRY s_xform_tests[] = {
	TEST_DECL(test_make_rotation),
	TEST_DECL(test_make_xform),
	TEST_DECL(test_xform_invert),
	TEST_DECL(test_3x4),
};

bool test_xform() {
	return 0 == EXEC_TESTS(s_xform_tests);
}