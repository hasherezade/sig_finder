#include "Match.h"
#include <sstream>
#include <fstream>

namespace sig_finder {

	size_t Match::saveMatches(const std::string& reportPath, const std::vector<Match>& matched_patterns, const char delimiter, size_t areaStartOffset)
	{
		if (matched_patterns.size() == 0) {
			return 0;
		}
		std::ofstream patch_report;
		patch_report.open(reportPath);
		if (patch_report.is_open() == false) {
			return 0;
		}
		size_t count = 0;
		for (auto itr = matched_patterns.begin(); itr != matched_patterns.end(); itr++) {
			sig_finder::Match m = *itr;
			patch_report << m.toTag(delimiter) << std::endl;
			count++;
		}
		if (patch_report.is_open()) {
			patch_report.close();
		}
		return count;
	}

	std::string Match::toTag(const char delimiter, size_t areaStartOffset)
	{
		std::stringstream ss;
		ss << std::hex << offset + areaStartOffset;
		ss << delimiter;
		ss << sign->name;
		ss << delimiter;
		ss << sign->size();
		return ss.str();
	}

};