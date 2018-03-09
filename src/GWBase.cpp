/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include "GWBase.hpp"

namespace GWBase {
	const long double pi = ::acos((long double)-1);

	GWRotationOrder rord_from_float(float val) {
		int ival = (int)val;
		return ival > (int)GWRotationOrder::MAX ? GWRotationOrder::XYZ : GWRotationOrder(ival);
	}

	GWTransformOrder xord_from_float(float val) {
		int ival = (int)val;
		return ival > (int)GWTransformOrder::MAX ? GWTransformOrder::SRT : GWTransformOrder(ival);
	}
}
