/*
* Author: Gleb Novodran <novodran@gmail.com>
*/

#include <fstream>
#include <cstddef>
#include "groundwork.hpp"

GWResource* GWResource::load(const std::string& path, const char* pSig) {
	char sig[0x10];

	std::ifstream fs(path, std::ios::binary);
	if (fs.bad()) { return nullptr; }

	fs.read(sig, 0x10);
	if (::memcmp(sig, GW_RSRC_SIG, sizeof(GW_RSRC_SIG) - 1) != 0) return nullptr;
	if (pSig) {
		if (::strcmp(sig, pSig) != 0) { return nullptr; }
	}

	fs.seekg(offsetof(GWResource, mDataSize));
	uint32_t size = 0;
	fs.read((char*)&size, 4);
	if (size < 0x10) return nullptr;
	char* pBuf = new char[size];
	if (pBuf) {
		fs.seekg(std::ios::beg);
		fs.read(pBuf, size);
	}

	fs.close();
	return reinterpret_cast<GWResource*>(pBuf);
}
