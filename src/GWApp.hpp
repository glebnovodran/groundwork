/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

namespace GWApp {
	void init(int argc, char* argv[]);
	void reset();

	const char* get_full_path();
	int get_options_num();
	const char* get_option(const char* pOptName);
	int get_int_option(const char* pOptName, int defVal);
	float get_float_option(const char* pOptName, float defVal);
	int get_argument_num();
	const char* get_argument(int idx);
} // GWApp
