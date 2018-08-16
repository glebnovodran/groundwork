/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include "GWBase.hpp"
#include "GWVector.hpp"
#include "GWSphere.hpp"

template<typename T> GWSphereBase<T> GWSphere::ritter(GWVectorBase<T>* pPts, int numPts) {
	GWSphereBase<T> sph;
	GWTuple3i imin, imax;
	GWTuple::fill(imin, 0);
	GWTuple::fill(imax, 0);

	for (int i = 0; i < numPts; ++i) {
		GWVectorBase<T> pnt = pPts[i];

		if (pnt.x < pPts[imin.x].x) imin.x = i;
		if (pnt.x > pPts[imax.x].x) imax.x = i;
		if (pnt.y < pPts[imin.y].y) imin.y = i;
		if (pnt.y > pPts[imax.y].y) imax.y = i;
		if (pnt.z < pPts[imin.z].z) imin.z = i;
		if (pnt.z > pPts[imax.z].z) imax.z = i;
	}

	GWVectorBase<T> vx = pPts[imax.x] - pPts[imin.x];
	GWVectorBase<T> vy = pPts[imax.y] - pPts[imin.y];
	GWVectorBase<T> vz = pPts[imax.z] - pPts[imin.z];
	T dx = vx.length_sq();
	T dy = vy.length_sq();
	T dz = vz.length_sq();

	int minPntIdx = imin.x;
	int maxPntIdx = imax.x;
	if (dy > dx && dy > dz) {
		minPntIdx = imin.y;
		maxPntIdx = imax.y;
	}
	if (dz > dx && dz > dy) {
		minPntIdx = imin.z;
		maxPntIdx = imax.z;
	}

	sph.c = (pPts[minPntIdx] + pPts[maxPntIdx]) * T(0.5f);
	sph.r = (pPts[maxPntIdx] - sph.c).length();

	for (int i = 0; i < numPts; ++i) {
		sph.enclose(pPts[i]);
	}

	for (int i = 0; i < numPts; ++i) {
		T sqd = (pPts[i] - sph.c).length();
		if (sqd > sph.r * sph.r) {
			sph.r = ::sqrt(sqd) + (sph.r* T(1.0e-5));
		}
	}
	return sph;
}

template GWSphereBase<float> GWSphere::ritter(GWVectorBase<float>* pPnts, int numPts);
template GWSphereBase<double> GWSphere::ritter(GWVectorBase<double>* pPnts, int numPts);
