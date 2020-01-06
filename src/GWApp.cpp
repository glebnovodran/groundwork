/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */
#include <string>
#include <vector>
#include <unordered_map>

#include "GWSys.hpp"
#include "GWApp.hpp"

using namespace std;

class CommandLine {
protected:
	std::string mAppPath;
	std::unordered_map<std::string, std::string> mOptMap;
	std::vector<std::string> mArgs;
public:
	CommandLine() {}

	void parse(int argc, char* argv[]) {
		mAppPath = string(argv[0]);
		for (int i = 1; i < argc; ++i) {
			string sarg(argv[i]);
			if (sarg.front() == '-') {
				size_t offs = sarg.find_first_of(':');
				if (offs != string::npos) {
					mOptMap[sarg.substr(1,offs-1)] = sarg.substr(offs+1);
					continue;
				}
			}
			mArgs.push_back(std::move(sarg));
		}
	}

	void reset() {
		mAppPath = "";
		mOptMap.clear();
		mArgs.clear();
	}

	int get_options_num() { return mOptMap.size(); }

	const char* get_option(const char* pOptName) {
		return mOptMap[string(pOptName)].c_str();
	}

	int get_int_option(const char* pOptName, int defVal) {
		const char* pOptVal = get_option(pOptName);
		return (pOptVal==nullptr) ? defVal : ::atoi(pOptVal);
	}

	float get_float_option(const char* pOptName, float defVal) {
		const char* pOptVal = get_option(pOptName);
		return (pOptVal==nullptr) ? defVal : float(::atof(pOptVal));
	}

	int get_argument_num() {return mArgs.size(); }

	const char* get_argument(int idx) { return mArgs[idx].c_str(); }
};

namespace GWApp {
	static CommandLine s_CmdLine;

	void init(int argc, char* argv[]) {
		s_CmdLine.parse(argc, argv);
 	}

	void reset() {
		s_CmdLine.reset();
	}

	int get_options_num() { return s_CmdLine.get_options_num(); }

	const char* get_option(const char* pOptName) { return s_CmdLine.get_option(pOptName); }

	int get_int_option(const char* pOptName, int defVal) { return s_CmdLine.get_int_option(pOptName, defVal); }

	float get_float_option(const char* pOptName, float defVal) { return s_CmdLine.get_float_option(pOptName, defVal); }

	int get_argument_num() { return s_CmdLine.get_argument_num(); }

	const char* get_argument(int idx) { return s_CmdLine.get_argument(idx); }
}
