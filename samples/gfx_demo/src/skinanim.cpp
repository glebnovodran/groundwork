/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <groundwork.hpp>

#include "draw.hpp"
#include "demo.hpp"

void SkinAnimDemo::init() {
	mDrawIfc = get_ogl_impl();
	mDrawIfc.init();
}
void SkinAnimDemo::loop() {
	mDrawIfc.begin(GWColorF(0.5f, 0.6f, 0.65f));
	mDrawIfc.end();
}
void SkinAnimDemo::reset() {
	mDrawIfc.reset();
}

void SkinAnimDemo::get_preferred_window_size(int& w, int& h) const {
	w = 1024;
	h = 768;
}