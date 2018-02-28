/*
 * Groundwork project : a test program
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <iostream>
#include "groundwork.hpp"

void test_basic() {
	float rad = GWBase::radians(271.0f);
	float deg = GWBase::degrees(rad);
}

void test_tuple() {
	using namespace std;
	GWTuple3f tupleA = { 0.0f, 1.0f, 2.0f };
	GWTuple3f tupleB = { 2.0f, 2.0f, 2.0f };
	GWTuple4f tupleC = {};
	GWTuple::div(tupleC, tupleA, tupleB);
	cout << tupleC[0] << endl;
	tupleC[3] = 1.0f;
}

void test_vec() {
	using namespace std;
	GWVectorF a(1.0f, 0.0f, 0.0f);
	GWVectorF b(0.0f, 1.0f, 0.0f);

	GWVectorF c;
	c.cross(a, b);
	GWVectorF d = 0.5f * (a + b);
	c.set(b);
	GWTuple3f tupleA = { 0.0f, 1.0f, 2.0f };
	GWTuple3f tupleB;
	GWTuple::set(tupleB, 1, 2, 3);

	GWTuple3f tuple3 = { 4, 5, 6 };
	GWTuple4d tuple4 = { 4, 5, 6, 7};
	GWVectorF fromTuple3(tuple3);
	GWVectorF fromTuple4;
	fromTuple4.from_tuple(tuple4);
	fromTuple4.normalize(fromTuple3);

	float res = a.dot(b);
	a.add(b);
	a.scl(0.5);
	float maxAbsElem = a.max_abs_elem();

	a.normalize();
	a = -a;
	float la = a.length();
	float laf = a.length_fast();
}

void test_quat() {
	using namespace std;
	GWQuaternionF q;
	q.set_identity();
	q.set_radians((float)GWBase::pi, 0.0f, (float)(GWBase::pi*0.5f));
	q.log();

	GWTuple4d tuple4 = { 1.0f, 0.0f, 1.0f, 0.5f };
	GWQuaternionD p;
	p.from_tuple(tuple4);
	p.normalize();
	p.conjugate();
	GWQuaternionF qf(p);

	GWQuaternionF t(1.0f, 2.0f, 3.0f, 0.5f);
	t.normalize();
	GWVectorF encoded = t.expmap_encode();
	q.expmap_decode(encoded);
	cout << "Geodesic distance with the decoded quaternion = " << t.arc_distance(q) << endl;
	q.normalize();

	GWQuaternionF q1(1.0f, 2.0f, 3.0f, 0.5f);
	GWQuaternionF p1 = -q1;
	float dist = GWQuaternion::arc_distance(q1, p1);
	cout << "Geodesic distance between inverse quaternions = " << dist << endl;
	GWVectorF vec(1.0f, 1.0f, 0.0f);
	GWVectorF vec0 = q1.apply(vec);
	q1.normalize();
	GWVectorF vec1 = q1.apply(vec);

	GWQuaternionF qq;
	qq.set_degrees(-0.106412f, -8.22023f, -0.852421f);
	qq.normalize();

	qq.set_radians(GWBase::pi / 4.0f, GWBase::pi / 8.0f, GWBase::pi / 4.0f);

	qq.normalize();
	GWVectorF radians = GWUnitQuaternion::get_radians(qq);
	qq.set_degrees(10.0f, 20.0f, 30.0f);
	GWVectorF degrees = GWUnitQuaternion::get_degrees(qq);
}

void test_motion() {
	using namespace std;
	GWMotion mot;
	if (mot.load("../data/row_names.txt")) {
		GWMotion::Node node = mot.get_node("/obj/j_Head");
		GWMotion::Track rotTrk = node.get_track(GWTrackKind::ROT);
		GWQuaternionF q = rotTrk.eval_quat(0.5f);
		GWVectorF deg = GWUnitQuaternion::get_degrees(q);
		uint32_t badId = 7777;
		node = mot.get_node_by_id(badId);
		rotTrk = node.get_track(GWTrackKind::ROT);
		if (rotTrk.is_valid()) { q = rotTrk.eval_quat(10.0f); }

		for (int i = 0; i < 100; ++i) {
			GWVectorF val = mot.eval(0, GWTrackKind::ROT, i + 0.5f);
			GWQuaternionF q = GWQuaternion::expmap_decode(val);
			q.normalize();
			cout << q.V().x << " " << q.V().y << " " << q.V().z << " " << q.S() <<endl;
		}

		node = mot.get_node("====");
	}
}

int main(int argc, char* argv[]) {

	test_basic();
	test_tuple();
	test_vec();
	test_quat();
	test_motion();

	return 0;
}
