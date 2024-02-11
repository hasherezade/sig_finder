#pragma once

#include "../win_types.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

namespace sig_finder {

	class Signature
	{
	public:
		static Signature* loadFromByteStr(const std::string& signName, const std::string& content);
		static size_t loadFromFile(std::string filename, std::vector<Signature*>& signatures);
		static size_t loadFromFileStream(std::ifstream& input, std::vector<Signature*>& signatures);

		Signature(std::string _name, const BYTE* _pattern, size_t _pattern_size, const BYTE* _mask)
			: name(_name), pattern(nullptr), pattern_size(0), mask(nullptr)
		{
			init(_name, _pattern, _pattern_size, _mask);
		}

		Signature(const Signature& _sign) // copy constructor
			: pattern(nullptr), pattern_size(0), mask(nullptr)
		{
			init(_sign.name, _sign.pattern, _sign.pattern_size, _sign.mask);
		}

		bool operator==(const Signature& rhs) const
		{
			if (this->pattern_size != rhs.pattern_size) {
				return false;
			}
			if (this->pattern && rhs.pattern) {
				if (::memcmp(pattern, rhs.pattern, pattern_size) != 0) {
					return false;
				}
			}
			if (mask && rhs.mask) {
				if (::memcmp(mask, rhs.mask, pattern_size) != 0) {
					return false;
				}
			}
			return true;
		}

		size_t size()
		{
			return pattern_size;
		}

		std::string toByteStr();

		std::string name;

	protected:
		size_t pattern_size;
		BYTE* pattern;
		BYTE* mask;

	private:
		bool init(std::string _name, const BYTE* _pattern, size_t _pattern_size, const BYTE* _mask)
		{
			if (this->pattern || this->mask) return false;

			this->pattern = (BYTE*)::calloc(_pattern_size, 1);
			if (!this->pattern) return false;

			::memcpy(this->pattern, _pattern, _pattern_size);
			this->pattern_size = _pattern_size;

			if (_mask) {
				this->mask = (BYTE*)::calloc(_pattern_size, 1);
				if (this->mask) {
					::memcpy(this->mask, _mask, _pattern_size);
				}
			}
			return true;
		}

		friend class Node;
	};

}; // namespace sig_finder
