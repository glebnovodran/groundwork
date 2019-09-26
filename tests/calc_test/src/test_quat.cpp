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

static bool test_quat_set_get_rad() {
	bool res = true;
	GWQuaternionF q(0.33527f, 0.28532f, 0.018283f, 0.897693f);
	GWVectorF rad(GWBase::pi/2.0f, GWBase::pi/6.0f, GWBase::pi/8.0f);
	//GWVectorF rad(0.0f, 0.0f, 0.0f);
	GWQuaternionF p;
	p.set_radians(rad.x, rad.y, rad.z);
	p.normalize();
	GWVectorF rad0 = GWUnitQuaternion::get_radians(p);
	res &= COMPARE_VEC(rad, rad0, 0.0001f);
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

static bool test_get_transform() {
	GWQuaternionF q;
	GWVectorF deg(45.0f, 30.0f, 15.0f);
	q.set_degrees(deg.x, deg.y, deg.z);
	GWTransformF xform;
	xform.make_rotation(q); // TODO: move to transform test
	float res[16] = {
		0.836516, 0.224144, -0.5, 0,
		0.158494, 0.774519, 0.612372, 0,
		0.524519, -0.591506, 0.612372, 0,
		0, 0, 0, 1
	};
	return compare_mtx<float>(xform.as_tptr(), res, 4, 4);
}

static bool test_to_transform() {
	GWQuaternionF q, p;
	q.set_degrees(15.0f, 30.0f, 60.0f);
	q.normalize();
	GWTransformF xform;
	xform.make_rotation(q);
	p = GWUnitQuaternion::from_transform(xform.as_tptr(), 4, true);
	p.normalize();
	float dist = GWUnitQuaternion::arc_distance(q, p);
	bool res = 0.0001f >= dist;
	GWTransform3x4F xform34;
	xform34.make_rotation(q);
	p = GWUnitQuaternion::from_transform(xform34.as_tptr(), 4, false);
	res = res && (0.0001f >= GWUnitQuaternion::arc_distance(q, p));
	return res;
}

static bool test_closest() {
	GWQuaternionF q;
	q.set_degrees(30.0f, 0.0f, 0.0f);
	q.normalize();
	GWQuaternionF cnstrd = GWUnitQuaternion::closest_by_axis(q, 0);
	float dist = q.arc_distance(cnstrd);
	if (dist>0.001f) return false;

	GWQuaternionF qq;
	qq.set_degrees(15.0f, -30.0f, 75.0f);
	q.set_degrees(15.0f, 0.0f, 0.0f);
	cnstrd = GWUnitQuaternion::closest_by_axis(q, 0);
	dist = q.arc_distance(cnstrd);
	if (dist>0.001f) return false;

	q.set_degrees(0.0f, -30.0f, 0.0f);
	cnstrd = GWUnitQuaternion::closest_by_axis(q, 1);
	dist = q.arc_distance(cnstrd);
	if (dist>0.001f) return false;

	q.set_degrees(0.0f, 0.0f, 75.0f);
	cnstrd = GWUnitQuaternion::closest_by_axis(q, 2);
	dist = q.arc_distance(cnstrd);
	if (dist>0.001f) return false;

	cnstrd = GWUnitQuaternion::closest_xy(q);
	cnstrd = GWUnitQuaternion::closest_yx(q);
	return true;
}

static TEST_ENTRY s_quat_tests[] = {
	TEST_DECL(test_quat_set_get),
	TEST_DECL(test_quat_set_get_rad),
	TEST_DECL(test_apply),
	TEST_DECL(test_quat_expmap),
	TEST_DECL(test_get_transform),
	TEST_DECL(test_to_transform),
	TEST_DECL(test_closest)
};

bool test_quat() {
	return 0 == EXEC_TESTS(s_quat_tests);
}
