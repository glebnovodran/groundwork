/*
 * Groundwork calculation test
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <iostream>
#include <groundwork.hpp>

#include "test.hpp"

#define PRINT_RESULT(res) cout << (res ? "OK" : "failed") << endl;

static TEST_ENTRY s_tests[] = {
	TEST_DECL(test_quat)
};

int main(int argc, char* argv[]) {
	using namespace std;
	int nfailed = EXEC_TESTS(s_tests);
	if (0 == nfailed) {
		cout << "\nTests passed" << endl;
	} else {
		cout << "\nTests failed" << endl;
	}
	return nfailed;
}
