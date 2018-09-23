/*
 * Groundwork calculation test utils
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#ifdef _WIN32
#	define WIN32_LEAN_AND_MEAN 1
#	define NOMINMAX
#	define _WIN32_WINNT 0x0500
#	include <Windows.h>
#endif

#include <time.h>

#include "test.hpp"

double time_micros() {
	double ms = 0.0f;
#if defined(_WIN32)
	LARGE_INTEGER frq;
	if (QueryPerformanceFrequency(&frq)) {
		LARGE_INTEGER ctr;
		QueryPerformanceCounter(&ctr);
		ms = ((double)ctr.QuadPart / (double)frq.QuadPart) * 1.0e6;
	}
#else
	struct timespec t;
	if (clock_gettime(CLOCK_MONOTONIC, &t) != 0) {
		clock_gettime(CLOCK_REALTIME, &t);
	}
	ms = (double)t.tv_nsec*1.0e-3 + (double)t.tv_sec*1.0e6;
#endif
	return ms;
}
