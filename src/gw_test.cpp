/*
 * Groundwork project : test program
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <iostream>

#include "GW.hpp"


int main(int argc, char* argv[]) {
	using namespace std;
	GWVectorF a(1.0f, 0.0f, 0.0f);
	GWVectorF b(0.0f, 1.0f, 0.0f);

	GWVectorF c = GWVectorF::cross(a, b);

	float res = a.dot(b);
	a.add(b);
	a.scl(0.5);
	float maxAbsElem = a.max_abs_elem();

	float la = a.length();
	float laf = a.length_fast();

	return 0;
}
