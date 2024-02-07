#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>

namespace sig_ma {
	namespace util {

		inline std::string to_hex(const uint8_t val, int width = 2)
		{
			std::stringstream ss;
			ss << std::setw(width) << std::setfill('0') << std::hex << (unsigned int)(val);
			return ss.str();
		}

	};
};
