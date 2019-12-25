/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include "groundwork.hpp"

GWBundle* GWBundle::create(const std::string& name, const std::string& dataPath) {
	GWBundle* pBdl = nullptr;
	const std::string catFilePath = dataPath + "/" + name + ".gwcat";

	GWCatalog* pCat = GWCatalog::load(catFilePath);
	if (pCat != nullptr) {
		pBdl = create(name, dataPath, pCat);
	} else {
		GWSys::dbg_msg("Error: Cannot load %s", catFilePath.c_str());
	}

	return pBdl;
}

GWBundle* GWBundle::create(const std::string& name, const std::string& dataPath, GWCatalog* pCat) {
	GWBundle* pBdl = new GWBundle();
	MdlRscList* pMdlLst = &pBdl->mMdlLst;
	ImgList* pImgLst = &pBdl->mImgLst;
	MotList* pMotLst = &pBdl->mMotLst;
	ColList* pColLst = &pBdl->mColLst;

	uint32_t numRes = pCat->mNum;
	//pBdl->mpName = GWBase::str_dup(name.c_str());
	pBdl->mName = name;
	uint32_t numMdl = pCat->get_kind_count(GWResourceKind::MODEL);

	for (uint32_t i = 0; i < numRes; ++i) {
		GWResourceKind kind = pCat->get_kind(i);
		const char* pName = pCat->get_name(i);
		const char* pFileName = pCat->get_file_name(i);
		std::string filePath = dataPath + pCat->get_file_name(i);
		switch (kind) {
			case GWResourceKind::MODEL: {
					GWModelResource* pMdlRsc = GWModelResource::load(filePath);
					if (pMdlRsc != nullptr) {
						pMdlLst->add(new GWListItem<GWModelResource>(pName, pMdlRsc));
					} else {
						GWSys::dbg_msg("Error loading model file %s", filePath.c_str());
					}
				}
				break;
			case GWResourceKind::DDS: {
					GWImage* pImg = GWImage::read_dds(filePath);
					if (pImg != nullptr) {
						pImgLst->add(new GWListItem<GWImage>(pName, pImg));
					} else {
						GWSys::dbg_msg("Error loading image file %s", filePath.c_str());
					}
				}
				break;
			case GWResourceKind::TDMOT: {
					GWMotion* pMot = new GWMotion();
					if (pMot->load(filePath)) {
						pMotLst->add(new GWListItem<GWMotion>(pName, pMot));
					} else {
						delete pMot;
						GWSys::dbg_msg("Error loading TD motion file %s", filePath.c_str());
					}
				}
				break;
			case GWResourceKind::COL_DATA: {
					GWCollisionResource* pColli = GWCollisionResource::load(filePath);
					if (pColli != nullptr) {
						pColLst->add(new GWListItem<GWCollisionResource>(pName, pColli));
					} else {
						GWSys::dbg_msg("Error loading cls file %s", filePath.c_str());
					}
				}
				break;
			default:
				GWSys::dbg_msg("Error: unknown resource type");
				break;
		}
	}
	pBdl->mpCat = pCat;
	return pBdl;
}

void GWBundle::purge_models() {
	for (MdlRscList::Itr itr = mMdlLst.get_itr(); !itr.end(); itr.next()) {
		GWModelResource* pRsc = itr.val();
		GWResource::unload(pRsc);
	}
	mMdlLst.purge();
}

void GWBundle::purge_images() {
	for (ImgList::Itr itr = mImgLst.get_itr(); !itr.end(); itr.next()) {
		GWImage* pImg = itr.val();
		if (pImg) { GWImage::free(pImg); }
	}
	mImgLst.purge();
}

void GWBundle::purge_motions() {
	for (MotList::Itr itr = mMotLst.get_itr(); !itr.end(); itr.next()) {
		GWMotion* pMot = itr.val();
		pMot->unload();
	}
	mMotLst.purge();
}

void GWBundle::purge_colli_data() {
	for (ColList::Itr itr = mColLst.get_itr(); !itr.end(); itr.next()) {
		GWCollisionResource* pRsc = itr.val();
		GWResource::unload(pRsc);
	}
}

void GWBundle::destroy(GWBundle* pBdl) {
	if (pBdl) {
		pBdl->purge_models();
		pBdl->purge_images();
		pBdl->purge_motions();
		pBdl->purge_colli_data();
		GWResource::unload(pBdl->mpCat);
		delete pBdl;
	}
}

GWRsrcRegistry* GWRsrcRegistry::create(const std::string& appPath, const std::string& relDataDir) {
	using namespace std;
	GWRsrcRegistry* pRgy = new GWRsrcRegistry();
	if (pRgy != nullptr) {
		size_t last = appPath.find_last_of('/');
		if (last == string::npos) {
			last = appPath.find_last_of('\\');
		}
		last = (last == string::npos) ? appPath.length() : last;
		pRgy->mDataPath = appPath.substr(0, last) + "/" + relDataDir;
	}
	return pRgy;
}

void GWRsrcRegistry::destroy(GWRsrcRegistry* pRgy) {
	if (pRgy != nullptr) {
		for (BundleList::Itr itr = pRgy->mBdlLst.get_itr(); !itr.end(); itr.next()) {
			GWBundle* pBdl = itr.val();
			GWBundle::destroy(pBdl);
		}
		pRgy->mBdlLst.purge();
		delete pRgy;
	}
}

void GWRsrcRegistry::unload_bundle(const std::string& name) {
	GWListItem<GWBundle>* pItem =  mBdlLst.find_first(name.c_str());
	if (pItem != nullptr) {
		GWBundle::destroy(pItem->mpVal);
		mBdlLst.remove(pItem);
		delete pItem;
	}
}

GWBundle* GWRsrcRegistry::load_bundle(const std::string& name) {
	GWBundle* pBdl = GWBundle::create(name, mDataPath);
	if (pBdl != nullptr) {
		mBdlLst.add(new GWListItem<GWBundle>(name.c_str(), pBdl));
	}
}