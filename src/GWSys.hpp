/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */
#include <cstddef>
namespace GWSys {
	void dbg_msg(const char* pFmt, ...);
	double time_micros();
	void* bin_load(const char* pPath, size_t* pSize = nullptr);
	char* txt_load(const char* pPath);
	void bin_free(void* pData);
};
