/*
 * Groundwork quaternion tests
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <groundwork.hpp>
#include "test.hpp"

static bool test_quat_set_get() {
	GWQuaternionF q(0.33527f, 0.28532f, 0.018283f, 0.897693f);
	GWQuaternionF p;
	GWVectorF deg(45.0f, 30.0f, 15.0f);
	p.set_degrees(deg.x, deg.y, deg.z);
	bool res = COMPARE_QUAT(q, p);
	p.set_degrees(deg.x, deg.y, deg.z, GWRotationOrder::YZX);
	p.normalize();
	GWVectorF deg0 = GWUnitQuaternion::get_degrees(p, GWRotationOrder::YZX);
	res &= COMPARE_VEC(deg, deg0, 0.0001f);
	return res;
}

static bool test_apply() {
	GWQuaternionF q(0.33527f, 0.28532f, 0.018283f, 0.897693f);
	GWVectorF v(0.836516, 0.224144, -0.5);
	GWVectorF vx(1.0f, 0.0f, 0.0f);
	GWVectorF rotated = q.apply(vx);
	return COMPARE_VEC(rotated, v, 0.001f);
}
static bool test_quat_expmap() {
	GWQuaternionF q;
	GWQuaternionF p(1.0f, 2.0f, 3.0f, 0.5f);
	p.normalize();
	GWVectorF encoded = p.expmap_encode();
	q.expmap_decode(encoded);
	return COMPARE_QUAT(q, p);
}

static TEST_ENTRY s_quat_tests[] = {
	TEST_DECL(test_quat_set_get),
	TEST_DECL(test_apply),
	TEST_DECL(test_quat_expmap)
};

bool test_quat() {
	return 0 == EXEC_TESTS(s_quat_tests);
}
