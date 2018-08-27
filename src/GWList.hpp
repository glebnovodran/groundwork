/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <cstdint>
template<typename T> struct GWListItem {
	GWBase::StrHash mNameHash;
	char* mpName;
	T* mpPrev;
	T* mpNext;

	GWListItem() : mNameHash(), mpName(nullptr), mpPrev(nullptr), mpNext(nullptr) {}
	void set_name(const char* pName) {
		mpName = pName;
		mNameHash.calculate(pName);
	}
};

template<typename ITEM_T> class GWNamedObjList {
protected:
	ITEM_T* mpHead;
	ITEM_T* mpTail;
	uint32_t mCount;

public:
	GWNamedObjList() {}
	~GWNamedObjList() {}

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

	ITEM_T* find(const char* pName);
	ITEM_T* find_next(const ITEM_T* pItem);
};
