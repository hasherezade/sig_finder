#pragma once

#include "../win_types.h"
#include <iostream>
#include <vector>
#include <cstring>

#include "Signature.h"
#include "Match.h"
#include "Node.h"

namespace sig_finder {

	inline size_t find_all_matches(Node& rootN, const BYTE* loadedData, size_t loadedSize, std::vector<Match>& allMatches)
	{
		if (!loadedData || !loadedSize) {
			return 0;
		}
		rootN.getMatching(loadedData, loadedSize, allMatches, false);
		return allMatches.size();
	}

	inline Match find_first_match(Node& rootN, const BYTE* loadedData, size_t loadedSize)
	{
		Match empty;
		if (!loadedData || !loadedSize) {
			return empty;
		}
		std::vector<Match> allMatches;
		rootN.getMatching(loadedData, loadedSize, allMatches, true);
		if (allMatches.size()) {
			auto itr = allMatches.begin();
			return *itr;
		}
		return empty;
	}

}; //namespace sig_finder
