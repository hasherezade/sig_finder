#pragma once
#include "Signature.h"

namespace sig_finder {

	class Match
	{
	public:
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

		size_t offset;
		Signature* sign;
	};

}; // namespace sig_finder
