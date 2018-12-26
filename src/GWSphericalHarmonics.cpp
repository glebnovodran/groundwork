/*
 * Spherical Harmonics
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include "GWSys.hpp"
#include "GWBase.hpp"
#include "GWVector.hpp"
#include "GWColor.hpp"
#include "GWImage.hpp"
#include "GWSphericalHarmonics.hpp"

template<typename T> void GWSHCoeffsBase<T>::calc_pano(const GWImage* pImg) {
	int w = pImg->get_width();
	int h = pImg->get_height();
	T da = T((2.0*GWBase::pi / w) * (GWBase::pi / h));
	T dx[1];
	T dy[1];
	T dz[1];

	T sum = T(0);
	T iw = T(1) / w;
	T ih = T(1) / h;

	T* pRawCoefs = as_raw();
	std::fill_n(pRawCoefs, N*3, T(0));
	T dirCoefs[N];

	for (int y = 0; y < h; ++y) {
		T v = T(1) - (y + T(0.5f)) * ih;
		T dw = da * std::sin(GWBase::pi * v);
		T inclination = (v - T(1)) * GWBase::pi;
		T sinI = std::sin(inclination);
		T cosI = std::cos(inclination);
		for (int x = 0; x < w; ++x) {
			T u = (x + T(0.5f)) * iw;
			T azimuth = u * T(2) * GWBase::pi;
			T sinA = std::sin(azimuth);
			T cosA = std::cos(azimuth);
			dx[0] = cosA * sinI;
			dy[0] = cosI;
			dz[0] = sinA * sinI;
			GWSH::vec_project_i<T>(dirCoefs, dx, dy, dz, 1);

			GWColorF pix = pImg->get_pixel(x, y);
			for (int i = 0; i < N; ++i) {
				T sw = dirCoefs[i] * dw;
				for (int j = 0; j < 3; ++j) {
					mCoef[i][j] += pix[j] * sw;
				}
			}
		}
	}
}

template void GWSHCoeffsBase<float>::calc_pano(const GWImage* pImg);
template void GWSHCoeffsBase<double>::calc_pano(const GWImage* pImg);

template<typename T> void GWSHCoeffsBase<T>::synth_pano(GWImage * pImg) const {
	int w = pImg->get_width();
	int h = pImg->get_height();
	T da = T((2.0*GWBase::pi / w) * (GWBase::pi / h));
	T dx;
	T dy;
	T dz;

	T sum = T(0);
	T iw = T(1) / w;
	T ih = T(1) / h;

	for (int y = 0; y < h; ++y) {
		T v = T(1) - (y + T(0.5f)) * ih;
		T dw = da * std::sin(GWBase::pi * v);
		T inclination = (v - T(1)) * GWBase::pi;
		T sinI = std::sin(inclination);
		T cosI = std::cos(inclination);
		for (int x = 0; x < w; ++x) {
			T u = (x + T(0.5f)) * iw;
			T azimuth = u * T(2) * GWBase::pi;
			T sinA = std::sin(azimuth);
			T cosA = std::cos(azimuth);
			dx = cosA * sinI;
			dy = cosI;
			dz = sinA * sinI;

			GWColorF clr;
			clr.from_tuple(synthesize(dx, dy, dz));
			clr.a = 1.0f;
			clr.clip_negative();
			pImg->set_pixel(x, y, clr);
		}
	}
}

template void GWSHCoeffsBase<float>::synth_pano(GWImage * pImg) const;
template void GWSHCoeffsBase<double>::synth_pano(GWImage * pImg) const;
