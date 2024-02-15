#pragma once
#include "Signature.h"
#include <vector>

namespace sig_finder {

	class Match
	{
	public:

		static size_t saveMatches(const std::string& reportPath, const std::vector<Match>& matched_patterns, const char delimiter, size_t areaStartOffset = 0);

		//

		Match()
			: offset(0), sign(nullptr)
		{
		}

		Match(size_t _offset, Signature* _sign)
			: offset(_offset), sign(_sign)
		{
		}

		Match(const Match& _match) // copy constructor
		{
			offset = _match.offset;
			sign = _match.sign;
		}

		std::string toTag(const char delimiter, size_t areaStartOffset = 0);

		size_t offset;
		Signature* sign;
	};

}; // namespace sig_finder
