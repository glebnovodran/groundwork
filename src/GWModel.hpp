/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

class GWModel {
private:
	GWModel() {}
public:
	GWModelResource* mpRsc;
	GWTransform3x4F* mpSkelXforms;
	GWTransform3x4F* mpSkinXforms;
	void* mpParamMem;
	void* mpExtMem;

	GWTransform3x4F mWorld;

public:
	static GWModel* create(GWModelResource* pMdr, const size_t paramMemSz = 0, const size_t extMemSz = 0);
};
