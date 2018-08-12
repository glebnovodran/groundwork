/*
* Author: Gleb Novodran <novodran@gmail.com>
*/

#define GW_RSRC_SIG "rsrc:"
#define GW_RSRC_ID(_name) GW_RSRC_SIG##_name

struct GWResource {
	char mSignature[0x10];
	uint32_t mVersion;
	uint32_t mDataSize;
	uint32_t mStrsTop;
	uint32_t mStrsSize;

	const char* get_str(uint32_t offs) const {
		if (offs < mStrsSize) {
			return reinterpret_cast<const char*>(this) + mStrsTop + offs;
		}
		return nullptr;
	}

	void* get_ptr(uint32_t offs) {
		if (offs < mDataSize) {
			return reinterpret_cast<char*>(this) + offs;
		}
		return nullptr;
	}

	static GWResource* load(const std::string& path, const char* pSig);
	static void unload(GWResource* pRsrc) {
		if (pRsrc) {
			delete[] reinterpret_cast<char*>(pRsrc);
		}
	}
};
