/*
 * Spherical Harmonics projection test
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <iostream>
#include <groundwork.hpp>
//void print_coefs(const GWSHCoeffsF)
void test(const std::string& panoPath) {
	using namespace std;

	ifstream ifs(panoPath, ios::binary);
	if (ifs.good()) {
		GWImage* pPanoImg = GWImage::read_dds(ifs);
		if (pPanoImg != nullptr) {
			GWSHCoeffsF coefs;
			coefs.calc_pano(pPanoImg);
			for (int i = 0; i < 9; ++i) {
				cout << i << " : ";
				cout << coefs(i).r << ", " << coefs(i).g << ", " << coefs(i).b << endl;
			}
		} else {
			cout << "Not a DDS file" << endl;
		}
		ifs.close();

	} else {
		cout << "Can't open the file" << endl;
	}
}

int main(int argc, char* argv[]) {
	using namespace std;

	if (argc > 1) {
		test(argv[1]);
	}

	return -1;
}
