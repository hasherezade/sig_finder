#include "pattern_tree.h"
#include <sstream>

#include <iostream>
#include <iomanip>

using namespace pattern_tree;

#define WILD_CHAR '?'

namespace pattern_tree {
	bool inline is_hex(char c)
	{
		if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) return true;
		return false;
	};

	char inline hex_char_to_val(char c)
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

	bool parseSigNode(const char chunk[3], BYTE& val, BYTE& vmask)
	{
		if (is_hex(chunk[0]) && is_hex(chunk[1])) {
			val = (hex_char_to_val(chunk[0]) << 4) | (hex_char_to_val(chunk[1]));
			vmask = 0xFF;
			return true;
		}
		if (chunk[0] == WILD_CHAR && chunk[1] == WILD_CHAR) {
			val = 0;
			vmask = 0;
			return true;
		}
		if (chunk[0] == WILD_CHAR || chunk[1] == WILD_CHAR) {
			if (chunk[1] == WILD_CHAR && is_hex(chunk[0])) {
				val = hex_char_to_val(chunk[0]) << 4;
				vmask = 0xF0;
				return true;
			}
			else if (chunk[0] == WILD_CHAR && is_hex(chunk[1])) {
				val = hex_char_to_val(chunk[1]);
				vmask = 0x0F;
				return true;
			}
		}
		std::cerr << "Invalid chunk supplied: " << std::hex << chunk[0] << " : " << chunk[1] << std::endl;
		return false;
	}
};

std::string Signature::toByteStr()
{
	std::stringstream ss;
	for (size_t i = 0; i < this->pattern_size; i++) {
		if (mask[i] == MASK_IMM) {
			ss << std::setw(2) << std::setfill('0') << std::hex << (unsigned int)(pattern[i]);
		}
		else if (mask[i] == MASK_PARTIAL1) {
			ss << WILD_CHAR;
			ss << std::setw(1) << std::setfill('0') << std::hex << (unsigned int)(pattern[i] & MASK_PARTIAL1);
		}
		else if (mask[i] == MASK_PARTIAL2) {
			ss << std::setw(1) << std::setfill('0') << std::hex << (unsigned int)((pattern[i] & MASK_PARTIAL2) >> 4);
			ss << WILD_CHAR;
		}
		else if (mask[i] == MASK_WILDCARD) {
			ss << WILD_CHAR << WILD_CHAR;
		}
		ss << " ";
	}
	return ss.str();
}
//---

Signature* Signature::loadFromByteStr(const std::string& signName, const std::string& content)
{
	if (!content.length()) return false;

	const size_t buf_max = content.length() / 2;
	BYTE* pattern = (BYTE*)::calloc(buf_max, 1);
	BYTE* mask = (BYTE*)::calloc(buf_max, 1);
	if (!pattern || !mask) return false;

	bool isOk = true;
	std::stringstream input(content);
	size_t indx = 0;
	// parse all the nodes one by one
	while(!input.eof() && indx < buf_max) {
		// parse all chunks from the line
		char chunk[3] = { 0, 0, 0 };
		input >> chunk[0];
		if (input.eof()) {
			break;
		}
		input >> chunk[1];
		if (!parseSigNode(chunk, pattern[indx], mask[indx])) {
			isOk = false;
			break;
		}
		indx++;
	}
	Signature* sign = nullptr;
	if (isOk) {
		sign = new Signature(signName, pattern, indx, mask);
	}
	free(pattern);
	free(mask);
	return sign;
}