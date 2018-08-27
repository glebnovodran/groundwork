/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <cstdint>
template<typename T> struct GWListItem {
	GWBase::StrHash mNameHash;
	char* mpName;
	GWListItem<T>* mpPrev;
	GWListItem<T>* mpNext;
	T* mpVal;

	GWListItem() : mNameHash(), mpName(nullptr), mpPrev(nullptr), mpNext(nullptr), mpVal(nullptr) {}
	GWListItem(const char* pName, T* pVal = nullptr) : mpPrev(nullptr), mpNext(nullptr) {
		set_name(pName);
		mpVal = pVal;
	}

	void set_name(const char* pName) {
		mpName = const_cast<char*>(pName);
		mNameHash.calculate(pName);
	}
};

template<typename ITEM_T> class GWNamedObjList {
protected:
	ITEM_T* mpHead;
	ITEM_T* mpTail;
	uint32_t mCount;

public:
	GWNamedObjList() : mpHead(nullptr), mpTail(nullptr), mCount(0) {}

	void link(ITEM_T* pItem) {
		if (pItem == nullptr) { return; }

		pItem->mpPrev = nullptr;
		pItem->mpNext = nullptr;

		if (mpHead == nullptr) {
			mpHead = pItem;
		} else {
			pItem->mpPrev = mpTail;
		}
		if (mpTail != nullptr) {
			mpTail->mpNext = pItem;
		}
		mpTail = pItem;
		++mCount;
	}

	void unlink(ITEM_T* pItem) {
		if (pItem == nullptr) { return; }

		ITEM_T pNext = pItem->mpNext;
		if (pItem->mpPrev == nullptr) {
			mpHead = pNext;
			if (pNext == nullptr) {
				mpTail = nullptr;
			} else {
				pNext->mpPrev = nullptr;
			}
		} else {
			pItem->mpPrev->mpNext = pNext;
			if (pNext == nullptr) {
				mpTail = pItem->mpPrev;
			} else {
				pNext->mpPrev = pItem->mpPrev;
			}
		}
		--mCount;
	}

	ITEM_T* find_first(const char* pName) {
		if (pName == nullptr) { return nullptr; }
		GWBase::StrHash nameHash(pName);
		return find(nameHash, pName);
	}

	ITEM_T* find_next(const ITEM_T* pFirst) {
		if (pFirst == nullptr) { return nullptr; }
		return find(pFirst->mNameHash, pFirst->mpName, pFirst);
	}

protected:
	inline ITEM_T* find(const GWBase::StrHash& nameHash, const char* pName, const ITEM_T* pFirst = nullptr) {
		ITEM_T* pFound = nullptr;
		ITEM_T* pItem = pFirst == nullptr? mpHead : pFirst->mpNext;

		while (pItem) {
			if (pItem->mNameHash == nameHash) {
				uint32_t len = pItem->mNameHash.len;
				bool equal = true;
				for (uint32_t i = 0; i < len; ++i) {
					if (pName[i] != pItem->mpName[i]) {
						equal = false;
						break;
					}
				}
				if (equal) {
					pFound = pItem;
					break;
				}
			}
			pItem = pItem->mpNext;
		}
		return pFound;
	}
};
