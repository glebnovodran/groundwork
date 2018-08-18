/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

struct GLDrawCfg {
	int width;
	int height;
};

namespace GLDraw {
	void init(const GLDrawCfg& cfg);
	void reset();
	void loop(void(*pLoop)());

	void set_view(const GWVectorF& pos, const GWVectorF& tgt, const GWVectorF& up = GWVectorF(0.0f, 1.0f, 0.0f));
	void set_degreesFOVY(float deg);
	void set_view_range(float znear, float zfar);

	void begin();
	void end();
}
