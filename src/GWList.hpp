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
		set_name_val(pName, pVal);
	}

	void set_name(const char* pName) {
		mpName = const_cast<char*>(pName);
		mNameHash.calculate(pName);
	}

	void set_name_val(const char* pName, T* pVal) {
		set_name(pName);
		mpVal = pVal;
	}
};

template<typename T> class GWNamedObjList {
public:
	class Itr {
		protected:
			GWListItem<T>* mpItem;
		public:
			Itr(GWListItem<T>* pItem) : mpItem(pItem) {}
			GWListItem<T>* item() { return mpItem; }
			T* val() { mpItem ? mpItem->mpVal : nullptr; }
			const char* name() const { mpItem ? mpItem->mpName : nullptr; }
			bool end() const { mpItem == nullptr; }
			void next() {
				if (mpItem) { mpItem = mpItem->mpNext; }
			}
			void back() {
				if (mpItem) { mpItem = mpItem->mpPrev; }
			}
	};
protected:
	GWListItem<T>* mpHead;
	GWListItem<T>* mpTail;
	uint32_t mCount;

public:
	GWNamedObjList() : mpHead(nullptr), mpTail(nullptr), mCount(0) {}

	uint32_t get_count() const { return mCount; }
	Itr get_itr() const { return Itr(mpHead); }

	void add(GWListItem<T>* pItem) {
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

	void remove(GWListItem<T>* pItem) {
		if (pItem == nullptr) { return; }

		GWListItem<T>* pNext = pItem->mpNext;
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

	void purge() {
		GWListItem<T>* pItem = mpHead;
		while (pItem) {
			GWListItem<T>* pNext = pItem->mpNext;
			delete pItem;
			pItem = pNext;
		}
	}

	GWListItem<T>* find_first(const char* pName) {
		if (pName == nullptr) { return nullptr; }
		GWBase::StrHash nameHash(pName);
		return find(nameHash, pName);
	}

	GWListItem<T>* find_next(const GWListItem<T>* pFirst) {
		if (pFirst == nullptr) { return nullptr; }
		return find(pFirst->mNameHash, pFirst->mpName, pFirst);
	}

protected:
	inline GWListItem<T>* find(const GWBase::StrHash& nameHash, const char* pName, const GWListItem<T>* pFirst = nullptr) {
		GWListItem<T>* pFound = nullptr;
		GWListItem<T>* pItem = pFirst == nullptr? mpHead : pFirst->mpNext;

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
