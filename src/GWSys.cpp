/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */
#include <cstdarg>
#include <iostream>
#include "GWSys.hpp"

void GWSys::dbg_msg(const char* pFmt, ...) {
	char buf[1024];
	va_list lst;
	va_start(lst, pFmt);
#ifdef _MSC_VER
	vsprintf_s(buf, sizeof(buf), pFmt, lst);
#else
	vsprintf(buf, pFmt, lst);
#endif
	va_end(lst);
	std::cout << buf;
}
