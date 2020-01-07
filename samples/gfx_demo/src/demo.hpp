/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

class DemoIfc {
protected:
	GWDraw::Interface mDrawIfc;
public:
	virtual void init() = 0;
	virtual void loop() = 0;
	virtual void reset() = 0;
	virtual void get_preferred_window_size(int& w, int& h) const = 0;
};

class SkinAnimDemo : public DemoIfc {
public:
	void init();
	void loop();
	void reset();
	void get_preferred_window_size(int& w, int& h) const;
};
