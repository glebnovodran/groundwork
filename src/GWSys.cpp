/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#ifdef _WIN32
#	define WIN32_LEAN_AND_MEAN 1
#	define NOMINMAX
#	define _WIN32_WINNT 0x0500
#	include <Windows.h>
#endif

//#include <time.h>
#include <iostream>
#include <cstdarg>
#include <chrono>

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
#ifdef _WIN32
	OutputDebugStringA(buf);
#elif defined(UNIX)
	std::cout << buf << std::endl;
#endif
}

double GWSys::time_micros() {
	using namespace std::chrono;
	auto t = high_resolution_clock::now();
	return (double)duration_cast<nanoseconds>(t.time_since_epoch()).count() * 1.0e-3;
}
