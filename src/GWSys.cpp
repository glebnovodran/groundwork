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

namespace GWSys {

	void* alloc_rsrc_mem(const size_t size) {
		return new char[size];
	}

	void free_rsrc_mem(void* pMem) {
		delete[] reinterpret_cast<char*>(pMem);
	}

	void* alloc_temp_mem(const size_t size) {
		return new char[size];
	}
	void free_temp_mem(void* pMem) {
		delete[] reinterpret_cast<char*>(pMem);
	}

	void dbg_msg(const char* pFmt, ...) {
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
#else
		std::cout << buf << std::endl;
#endif
	}

	double time_micros() {
		using namespace std::chrono;
		auto t = high_resolution_clock::now();
		return (double)duration_cast<nanoseconds>(t.time_since_epoch()).count() * 1.0e-3;
	}

	void* load_impl(const char* pPath, size_t* pSize, bool asText) {
		void* pData = nullptr;
		size_t size = 0;
		FILE* pFile = nullptr;
		const char* pMode = "rb";
#if defined(_MSC_VER)
		fopen_s(&pFile, pPath, pMode);
#else
		pFile = fopen(pPath, pMode);
#endif
		if (pFile) {
			if (fseek(pFile, 0, SEEK_END) == 0) {
				size = ftell(pFile);
			}
			fseek(pFile, 0, SEEK_SET);
			if (size) {
				if (asText) ++size;
				pData = alloc_rsrc_mem(size);
				if (pData) {
					fread(pData, 1, size, pFile);
					if (asText) {
						((char*)pData)[size - 1] = 0;
					}
				}
			}
			fclose(pFile);
		}
		if (pSize) {
			*pSize = size;
		}
		return pData;
	}

	void free_impl(void* pData) {
		free_rsrc_mem(pData);
	}
	void* bin_load(const char* pPath, size_t* pSize) {
		return load_impl(pPath, pSize, false);
	}

	char* txt_load(const char* pPath) {
		return (char*)load_impl(pPath, nullptr, true);
	}

	void bin_free(void* pData) {
		free_impl(pData);
	}
}
