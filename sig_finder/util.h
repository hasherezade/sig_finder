#pragma once
#include <ctype.h>

#include <iostream>
#include <sstream>
#include <iomanip>

namespace sig_finder {

	namespace util {

		inline std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
		{
			str.erase(0, str.find_first_not_of(chars));
			return str;
		}

		inline std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
		{
			str.erase(str.find_last_not_of(chars) + 1);
			return str;
		}

		inline std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
		{
			return ltrim(rtrim(str, chars), chars);
		}

		inline bool is_hex(char c)
		{
			if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) return true;
			return false;
		};

		inline char hex_char_to_val(char c)
		{
			if (c >= '0' && c <= '9') {
				return c - '0';
			}
			if (c >= 'a' && c <= 'f') {
				return c - 'a' + 10;
			}
			if (c >= 'A' && c <= 'F') {
				return c - 'A' + 10;
			}
			return 0;
		};
	}
};
