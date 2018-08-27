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
};

template<typename ITEM_T> class GWNamedObjList {
public:
	GWNamedObjList() {}
	~GWNamedObjList() {}

	void add(ITEM_T* pItem) {
		insert_before(mpTop);
	}
	void remove(ITEM_T* pItem) {
		if (pItem->mpPrev) { pItem->mpPrev->mpNext = pItem->mpNext; }
		if (pItem->mpNext) { pItem->mpNext->mpPrev = pItem->mpPrev; }
	}

	ITEM_T* find(const char* pName);
	ITEM_T* find_next(const ITEM_T* pItem);
protected:

	void insert_after(ITEM_T* pWhere, ITEM_T* pItem) {
		pItem->mpPrev = pWhere;
		pItem->mpNext = pWhere->mpNext;
		pWhere->mpNext = pItem;
		if (pItem->mpNext != nullptr) {
			pItem->mpNext->mpPrev = pItem;
		}
	}

	void insert_before(ITEM_T* pWhere, ITEM_T* pItem) {
		pItem->mpPrev = pWhere->mpPrev;
		pItem->mpNext = pWhere;
		pWhere->mpPrev = pItem;
		if (pItem->mpPrev != nullptr) {
			pItem->mpPrev->mpNext = pItem;
		}
	}

protected:
	ITEM_T* mpTop;
private:

};
