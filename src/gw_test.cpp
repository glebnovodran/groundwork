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
	GWTuple3f tupleA = { 0.0f, 1.0f, 2.0f };
	GWTuple3f tupleB = { 2.0f, 2.0f, 2.0f };
	GWTuple4f tupleC = {};
	GWTuple::div(tupleC, tupleA, tupleB);
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

	float res = a.dot(b);
	a.add(b);
	a.scl(0.5);
	float maxAbsElem = a.max_abs_elem();

	a.normalize();

	float la = a.length();
	float laf = a.length_fast();
}

void test_quat() {
	GWQuaternionF q;
	q.identity();
	q.set_radians((float)GWBase::pi, 0.0f, (float)(GWBase::pi*0.5f));

	GWTuple4d tuple4 = { 1.0f, 0.0f, 1.0f, 0.5f };
	GWQuaternionD p;
	p.from_tuple(tuple4);
	p.normalize();
}

int main(int argc, char* argv[]) {
	test_basic();
	test_tuple();
	test_vec();
	test_quat();
	return 0;
}
