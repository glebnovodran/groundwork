/*
 * Groundwork project : a test program
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <iostream>
#include "groundwork.hpp"

void test_basic() {
	using namespace std;
	float rad = GWBase::radians(271.0f);
	float deg = GWBase::degrees(rad);
	GWRotationOrder rord = GWBase::rord_from_float(75.0f);

	GWBase::Random rnd;
	rnd.set_seed(32);
	uint64_t rndVal = GWBase::random_u64();
	for (int i = 0; i < 100; ++i) {
		rndVal = rnd.u64();
	}

	GWVectorF v(-2.0f, 2.5f, -1.5f);
	v.normalize();
	float mag = GWTuple::magnitude(v);

	float ox, oy;
	GWVectorF decoded;
	GWBase::vec_to_oct(v.x, v.y, v.z, ox, oy);
	GWBase::oct_to_vec(ox, oy, decoded.x, decoded.y, decoded.z);

	if (!GWTuple::compare(v, decoded, 0.001f)) {
		cout << "octo encoding diff test failed" << endl;
	}

	GWBase::StrHash hashA, hashB("bbb");
	cout << hashA.len << ":" << hashA.hash << endl;
	hashA.calculate("aaa");
	//hashB.calculate("aaa");
	cout << "compare " << hashA.val << " and " << hashB.val << " " << ((hashA == hashB) ? "true" : "false") << endl;
	cout << hashA.len << ":" << hashA.hash << endl;
	cout << hashB.len << ":" << hashB.hash << endl;
	cout << "=====================" << endl;

	float orig = -12.1f;
	float dec = 0.0f;
	uint16_t half;
	GWBase::float_to_half(&half, &orig, 1);
	GWBase::half_to_float(&dec, &half, 1);

	GWHalf h;
	GWHalf3 h3;
	h3.set((GWTuple3f)v);
	GWVectorF v1 = h3.get();
	cout << v.x << v.y << v.z << endl;
	cout << v1.x << v1.y << v1.z << endl;
	GWHalf4 h4;

	cout << "=====================" << endl;

}

void test_list() {
	using namespace std;
	int val0 = 0;
	int val1 = 1;
	int val2 = 2;

	GWListItem<int> item0("item0", &val0);
	GWListItem<int> item1;
	item1.set_name_val("item1", &val1);
	GWListItem<int> item2("item0", &val2);

	GWNamedObjList<int> list;
	list.add(&item0);
	list.add(&item1);
	list.add(&item2);

	GWListItem<int>* pFound = list.find_first("item1");
	pFound = list.find_first("item0");
	while (pFound) {
		cout << pFound->mpName << " : " << *pFound->mpVal << endl;
		pFound = list.find_next(pFound);
	}
	pFound = list.find_next(pFound);
	pFound = list.find_first("/root/item0");
	cout << "=====================" << endl;
	list.remove(&item1);
	pFound = list.find_first("item1");
	if (pFound != nullptr) { cout << "GWNamedObjist::remove failed" << endl;}
}

bool test_solve3() {
	static float A[] = {
		2, -1, 2,
		1, -2, 1,
		3, -1, 2
	};
	static float b[] = {
		10, 8, 11
	};

	float tmp[3];
	int perm[3];
	float LU[3 * 3];
	GWMatrix::lu_decomp(LU, A, 3, tmp, perm);
	float ans[3];
	GWMatrix::lu_solve(ans, LU, 3, perm, b);

	static float expected[] = { 1, -2, 3 }; // A \ b
	for (int i = 0; i < 3; ++i) {
		int d = GWBase::f32_ulp_diff(ans[i], expected[i]);
		if (d > 200) return false;
	}

	float inv[3 * 3];
	for (int i = 0; i < 3; ++i) {
		float v[3];
		for (int j = 0; j < 3; ++j) {
			v[j] = float(i == j ? 1 : 0);
		}
		GWMatrix::lu_solve(&inv[i * 3], LU, 3, perm, v);
	}
	GWMatrix::transpose(inv, 3);

	float invExpected[] = { // inv(A)
		-1, 0, 1,
		0.33333f, -0.66667f, 0,
		1.66667, -0.33333, -1
	};
	for (int i = 0; i < 3*3; ++i) {
		int d = GWBase::f32_ulp_diff(inv[i], invExpected[i]);
		if (d > 200) return false;
	}

	return true;
}

bool test_distmtx() {
	const int N = 1000;
	float* pMtx = new float[N * N];
	float* pLU = new float[N * N];
	int* pPerm = new int[N];
	float* pTmpVec = new float[N];
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < N; ++j) {
			int offs = i*N + j;
			pMtx[offs] = ::sqrt(float(i*i) + float(j*j));
		}
	}
	int dsgn = 0;
	bool res = GWMatrix::lu_decomp(pLU, pMtx, N, pTmpVec, pPerm, &dsgn);
	return res;
}

void test_mtx() {
	using namespace std;

	cout << "test_solve3...";
	bool res = test_solve3();
	cout << (res ? " OK " : "Failed") << endl;

	cout << "test_distmtx...";
	res = test_distmtx();
	cout << (res ? " OK " : "Failed") << endl;
}

void test_tuple() {
	using namespace std;
	GWTuple3f tupleA = { 0.0f, 1.0f, 2.0f };
	GWTuple3f tupleB = { 2.0f, 2.0f, 2.0f };
	GWTuple4f tupleC = {};
	GWTuple::div(tupleC, tupleA, tupleB);
	cout << tupleC[0] << endl;
	tupleC[3] = 1.0f;

	GWTuple::copy(tupleC, tupleB, 1.0f);

	GWColorTuple4f clr4;
	GWColorTuple3<double> clr3;
	GWTuple::fill(clr4, 0.0f);
	clr3.r = 1.0; clr3.g = 0.5; clr3.b = 0.25;
	clr4 = clr3;
	GWTuple::fill(clr4, 0.0f);
	clr3 = clr4;
	GWVectorF v(1.5, 0.5, -0.2);
	clr4 = v;
	GWColorF c;
	c = clr4;
	uint32_t encC = c.encode_rgba8();
	GWTuple::saturate(clr4);

	cout << "=====================" << endl;
}

void test_vec() {
	using namespace std;
	GWVectorF a(1.0f, 0.0f, 0.0f);
	GWVectorF b(0.0f, 1.0f, 0.0f);

	GWVectorF c;
	c.cross(a, b);
	GWVectorF d = 0.5f * (a + b);
	c.set(b);
	GWTuple3f tupleA = { 0.0f, 1.0f, 2.0f };
	GWTuple3f tupleB;
	GWTuple::set(tupleB, 1, 2, 3);

	GWTuple3f tuple3 = { 4, 5, 6 };
	GWTuple4d tuple4 = { 4, 5, 6, 7};
	GWVectorF fromTuple3(tuple3);
	GWVectorF fromTuple4;
	fromTuple4.from_tuple(tuple4);
	fromTuple4.normalize(fromTuple3);

	float res = a.dot(b);
	a.add(b);
	a.scl(0.5);
	float maxAbsElem = a.max_abs_elem();

	a.normalize();
	a = -a;
	float la = a.length();
	float laf = a.length_fast();
}

void test_ray() {
	using namespace std;
	GWRayF rayA(GWVectorF(0.0f), GWVectorF(1.0f, 0.0f, 0.0f));
	GWVectorF v = rayA.at(2.0f);

	GWRayF ray;
	ray.from_asimuth_inclination(0.0f, GWBase::pi * 0.5f);
	cout << "from_asimuth_inclination(0, 90) = (" << ray.direction().x << ", "<< ray.direction().y << ", "<< ray.direction().z << ")\n";
	cout << "=====================" << endl;
}

void test_xform() {
	using namespace std;
	GWTransformF xform = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		1, 1, 1, 1
	};
	GWTransformF xform0;
	xform0.transpose(xform);
	GWVectorF v(1.0f, 1.0f, 1.0f);
	GWVectorF res = xform.calc_vec(v);
	res = xform.calc_pnt(v);
	xform.transpose();

	GWTransformF sclX;
	sclX.make_scaling(1, 2, 3);
	GWTransformF trnX;
	trnX.make_translation(4, 5, 6);

	xform.set_identity();
	xform = GWXform::concatenate(sclX, trnX);

	GWTransform3x4F x34 = GWXformCvt::get_3x4(xform);
	GWTransformF x44 = GWXformCvt::get_4x4(x34);

	GWTransformF xform1 = {
		1, 0, 0, 0,
		0, 2, 0, 0,
		0, 0, 3, 0,
		3, 2, 1, 1
	};
	GWVectorF c(1.0f, 0.0f, 0.0f);
	GWVectorF a = xform1.calc_pnt(c);
	GWTransformF xform2 = xform1.get_inverted();
	GWVectorF b = xform2.calc_pnt(a);
	if (!GWTuple::compare(c, b, 0.001f)) {
		std::cout << "Inversion error\n";
	}
	xform2.invert();
	xform.invert();

	GWTransformF xformT;
	xformT.set_identity();
	xformT.set_translation(2.0f, 0.0f, 0.0f);
	xformT.invert();

	xform1.make_rotation(GWBase::radians(10.0f), GWBase::radians(20.0f), GWBase::radians(30.0f));
	GWQuaternionF q;
	q.set_degrees(10.0f, 20.0f, 30.0f);
	xform2.make_rotation(q);
	if (!xform1.compare(xform2, 0.0001f)) {
		std::cout << "make_rotation error\n";
	} else {
		std::cout << "make_rotation : ok" << endl;
	}

	GWTransformF rx; rx.make_deg_rx(10.0f);
	GWTransformF ry; ry.make_deg_ry(20.0f);
	GWTransformF rz; rz.make_deg_rz(30.0f);
	xform = rx;
	xform.apply(ry);
	xform = GWXform::concatenate(rx, ry);
	cout << "=====================" << endl;
}

void test_quat() {
	using namespace std;
	GWQuaternionF q;
	q.set_identity();
	q.set_radians((float)GWBase::pi, 0.0f, (float)(GWBase::pi*0.5f));
	q.log();

	GWTuple4d tuple4 = { 1.0, 0.0, 1.0, 0.5 };
	GWQuaternionD p;
	p.from_tuple(tuple4);
	p.normalize();
	p.conjugate();
	GWQuaternionF qf(p);

	GWQuaternionF t(1.0f, 2.0f, 3.0f, 0.5f);
	t.normalize();
	GWVectorF encoded = t.expmap_encode();
	q.expmap_decode(encoded);
	cout << "Geodesic distance with the decoded quaternion = " << t.arc_distance(q) << endl;
	q.normalize();

	GWQuaternionF q1(1.0f, 2.0f, 3.0f, 0.5f);

	GWVectorF vec(1.0f, 1.0f, 0.0f);
	GWVectorF vec0 = q1.apply(vec);
	q1.normalize();
	GWVectorF vec1 = q1.apply(vec);

	GWQuaternionF qq;
	qq.set_degrees(-0.106412f, -8.22023f, -0.852421f);
	qq.normalize();

	qq.set_radians(GWBase::pi / 4.0f, GWBase::pi / 8.0f, GWBase::pi / 4.0f);

	qq.normalize();
	GWVectorF radians = GWUnitQuaternion::get_radians(qq);
	qq.set_degrees(10.0f, 20.0f, 30.0f);
	GWVectorF degrees = GWUnitQuaternion::get_degrees(qq);

	GWQuaternionF qx;
	GWQuaternionF qy;
	qx.set_degrees(30.0f, 30.0f, 0.0f);
	qy.set_degrees(60.0f, 60.0f, 0.0f);

	qq = GWUnitQuaternion::slerp(qx, qy, 0.5f);
	qq.log();
	GWQuaternionF q3(0.0f, 0.0f, 0.0f, 0.5f);
	GWQuaternionF q2(qq);
	q2.mul(q3);
	qq.scl(0.5f);

	q.set_degrees(45, 30, 15);
	GWTransformF xform;
	xform.make_rotation(q);
	GWVectorF v(1.0f, 0.0f, 0.0f);
	GWVectorF v1 = q.apply(v);
	GWVectorF v2 = xform.calc_vec(v);
	cout << "=====================" << endl;
}

void test_motion(const std::string& motPath) {
	using namespace std;
	GWMotion mot;
	if (mot.load(motPath)) {
		GWMotion::Node node = mot.get_node("/obj/ANIM/j_Ankle_L");
		GWMotion::Track rotTrk = node.get_track(GWTrackKind::ROT);
		GWQuaternionF q = rotTrk.eval_quat(0.5f);
		GWVectorF deg = GWUnitQuaternion::get_degrees(q);
		GWTransformF xform;
		node.eval_xform(xform, 74.5f);
		node.eval_xform(xform, -0.5f);
		node.eval_xform(xform, -2.1f);

		GWQuaternionF lerpVal = node.eval_rot(21.3, false);
		GWQuaternionF slerpVal = node.eval_rot(21.3, true);
		cout << "slerp to exp lerp distance " << slerpVal.arc_distance(lerpVal) << endl;

		uint32_t badId = 7777;
		node = mot.get_node_by_id(badId);
		rotTrk = node.get_track(GWTrackKind::ROT);
		if (rotTrk.is_valid()) { q = rotTrk.eval_quat(10.0f); }
		for (int i = -10; i < 100; ++i) {
			GWVectorF val = mot.eval(0, GWTrackKind::ROT, i + 0.5f);
			GWQuaternionF q = GWQuaternion::expmap_decode(val);
			q.normalize();
			cout << q.V().x << " " << q.V().y << " " << q.V().z << " " << q.S() <<endl;
		}

		cout << "===========================================" << endl;
		for (uint32_t id = 0; id < mot.num_nodes(); ++id) {
			GWMotion::Node node = mot.get_node_by_id(id);
			cout << "Node: " << node.name() << endl;
			GWMotion::Track track = node.get_track(GWTrackKind::ROT);
			cout << "Rot : ";
			if (track.is_valid()) {
				const GWMotion::TrackInfo* pInfo = track.get_track_info();
				cout << " srcMask = " << (uint32_t)pInfo->srcMask;
				cout << "; dataMask = " << (uint32_t)pInfo->dataMask;
			} else {
				cout << "None";
			}
			cout << endl;

			track = node.get_track(GWTrackKind::TRN);
			cout << "Trn : ";
			if (track.is_valid()) {
				const GWMotion::TrackInfo* pInfo = track.get_track_info();
				cout << " srcMask = " << (uint32_t)pInfo->srcMask;
				cout << "; dataMask = " << (uint32_t)pInfo->dataMask;
			} else {
				cout << "None";
			}
			cout << endl;

			track = node.get_track(GWTrackKind::SCL);
			cout << "Scl : ";
			if (track.is_valid()) {
				const GWMotion::TrackInfo* pInfo = track.get_track_info();
				cout << " srcMask = " << (uint32_t)pInfo->srcMask;
				cout << "; dataMask = " << (uint32_t)pInfo->dataMask;
			} else {
				cout << "None";
			}
			cout << endl;
			cout << "-------------------------------------------" << endl;
		}
		node = mot.get_node("====");
		mot.save_clip("../data/dump.clip", GWMotion::RotDumpKind::DEG);
		GWMotion clonedMot;
		clonedMot.clone_from(mot);
		node = mot.get_node("/obj/ANIM/j_Ankle_R");
		xform.set_zero();
		node.eval_xform(xform, 10.0f);
		q = node.eval_rot(10.0f);
		node = clonedMot.get_node("/obj/ANIM/j_Ankle_R");
		node.eval_xform(xform, 10.0f);
		q = node.eval_rot(10.0f);

		GWMotion::TrackInfo* pTrk = const_cast<GWMotion::TrackInfo*>(mot.get_track_info(0));
		GWVectorF newRot[75] = {};
		newRot[1].y = 1.0f;
		pTrk->replace_data(newRot);

		mot.unload();
	} else {
		cout << "Couldn't load the motion file" << endl;
	}
}

void test_image(const std::string& imgPath) {
	using namespace std;
	ifstream ifs(imgPath, ios::binary);
	if (ifs.good()) {
		GWImage* pImg = GWImage::read_dds(ifs);
		if (pImg) {
			cout << "read " << pImg->get_width() << "x" << pImg->get_height() << '\n';
			cout << "min: " << pImg->get_min() << '\n';
			cout << "max:" << pImg->get_max() << '\n';

			ofstream os("out.dds", ios::binary);
			if (os.good()) {
				pImg->write_dds(os);
				os.close();
			}
		} else {
			cout << "Bad file format\n";
		}
		ifs.close();
	} else {
		cout << "Image file not found\n";
	}
}

void test_model_recource(const std::string& mdlPath) {
	using namespace std;

	GWModelResource* pMdlRsc = GWModelResource::load(mdlPath);
	if (pMdlRsc == nullptr) {
		cout << "Cannot load the model file" << endl;
		return;
	}
	GWModelResource::Material* pMtl = pMdlRsc->get_mtl(0);
	pMdlRsc->save_geo("out.geo");
}

void test_gwcat(const char* pPath) {
	GWCatalog* pCat = GWCatalog::load(pPath);
	if (!pCat) return;
	int n = pCat->mNum;
	for (int i = 0; i < n; ++i) {
		const char* pName = pCat->get_name(i);
		const char* pFileName = pCat->get_file_name(i);
		GWResourceKind kind = pCat->get_file_kind(i);
		std::cout << i << ": " << pName << " : " << pFileName << " , " << GWResourceUtil::get_kind_string(kind) << std::endl;
	}
}

int main(int argc, char* argv[]) {

	test_basic();
	test_list();
	test_tuple();
	test_mtx();
	test_vec();
	test_ray();
	test_xform();
	test_quat();
	test_motion("../data/walk_rn.txt");

	test_image("../data/pano_test1_h.dds");
	test_model_recource("../data/cook_rb/cook_rb.gwmdl");
	test_gwcat("../data/cook_rb/cook_rb.gwcat");

	return 0;
}
