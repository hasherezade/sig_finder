#pragma once

#include "../win_types.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

#include "CRC32.h"

namespace sig_finder {

	class Signature
	{
	public:
		static Signature* loadFromByteStr(const std::string& signName, const std::string& content);
		static size_t loadFromFile(std::string filename, std::vector<Signature*>& signatures);
		static size_t loadFromFileStream(std::ifstream& input, std::vector<Signature*>& signatures);

		Signature(const std::string& _name, const BYTE* _pattern, size_t _pattern_size, const BYTE* _mask = nullptr)
			: name(_name), pattern(nullptr), pattern_size(0), mask(nullptr), crc32(0)
		{
			init(_name, _pattern, _pattern_size, _mask);
		}

		Signature(const Signature& _sign) // copy constructor
			: pattern(nullptr), pattern_size(0), mask(nullptr), crc32(0)
		{
			init(_sign.name, _sign.pattern, _sign.pattern_size, _sign.mask);
		}

		bool operator==(const Signature& rhs) const
		{
			if (this->pattern_size != rhs.pattern_size) {
				return false;
			}
			return (this->crc32 == rhs.crc32) ? true : false;
		}

		bool operator!=(const Signature& rhs) const
		{
			return (*this == rhs) ? false : true;
		}

		size_t size()
		{
			return pattern_size;
		}

		DWORD checksum()
		{
			return this->crc32;
		}

		std::string toByteStr();

		std::string name;

	protected:
		size_t pattern_size;
		BYTE* pattern;
		BYTE* mask;
		DWORD crc32;

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
			calcCrc32();
			return true;
		}

		bool calcCrc32()
		{
			if (!pattern) return false;

			util::CRC32 crcCalc;
			crcCalc.update(pattern, pattern_size);
			if (mask) {
				crcCalc.update(mask, pattern_size);
			}
			else {
				for (size_t i = 0; i < pattern_size; i++) {
					crcCalc.update(0xFF); //IMM
				}
			}
			this->crc32 = crcCalc.getChecksum();
			return true;
		}

		friend class Node;
	};

}; // namespace sig_finder
