/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */
#include <iostream>

#include "GWBase.hpp"
#include "GWColor.hpp"

std::ostream& operator << (std::ostream & os, const GWColorF & color) {
	os << "(" << color.r << ", " << color.g << ", " << color.b << ", " << color.a << ")";
	return os;
}
