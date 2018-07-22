/*
 * Spherical Harmonics projection test
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <iostream>
#include <groundwork.hpp>

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

			GWImage* pSynthImg = GWImage::alloc(pPanoImg->get_width(), pPanoImg->get_height());
			//coefs.synth_pano(pSynthImg);
			ofstream os("_synth.dds", ios::binary);
			if (os.good()) {
				coefs.synth_pano(pSynthImg);
				pSynthImg->write_dds(os);
				os.close();
			}

			GWSHCoeffsF coefsIrr = coefs;
			float weights[3];
			GWSH::calc_irradiance_weights(weights, 1.0f);
			coefsIrr.apply_weights(weights);

			os.open("_irr.dds", ios::binary);
			if (os.good()) {
				coefsIrr.synth_pano(pSynthImg);
				pSynthImg->write_dds(os);
				os.close();
			}

			GWSHCoeffsF coefsRefl = coefs;
			GWSH::calc_phong_weights(weights, 2.5f, 1.0f);
			coefsRefl.apply_weights(weights);
			os.open("_phong.dds", ios::binary);
			if (os.good()) {
				coefsRefl.synth_pano(pSynthImg);
				pSynthImg->write_dds(os);
				os.close();
			}
		} else {
			cout << "Not a DDS file" << endl;
		}
		ifs.close();

	} else {
		cout << "Can't open the file" << endl;
	}
}

void print_usage() {
	using namespace std;

	cout<<"sh_pano_test <dds file name>" << endl;
}

int main(int argc, char* argv[]) {
	using namespace std;

	if (argc > 1) {
		test(argv[1]);
	} else {
		print_usage();
	}

	return -1;
}
