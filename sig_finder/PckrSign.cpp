#include "PckrSign.h"

#include <iomanip>
#include <sstream>
#include "util.h"

bool sig_ma::PckrSign::parseSigNode(char chunk[3])
{
	sig_type node_type = sig_type::ROOT;
	unsigned int val = 0;
	uint8_t vmask = 0xFF;
	if (is_hex(chunk[0]) && is_hex(chunk[1])) {
		node_type = IMM;
		val = (hex_char_to_val(chunk[0]) << 4) | (hex_char_to_val(chunk[1]));
	}
	else if (chunk[0] == WILD_ONE && chunk[1] == WILD_ONE) {
		node_type = WILDC;
		val = chunk[0];
		vmask = 0;
	}
	else if (chunk[0] == WILD_ONE || chunk[1] == WILD_ONE) {
		node_type = PARTIAL;
		if (chunk[1] == WILD_ONE && is_hex(chunk[0])) {
			val = hex_char_to_val(chunk[0]) << 4;
			vmask = 0xF0;
		}
		else if (chunk[0] == WILD_ONE && is_hex(chunk[1])) {
			val = hex_char_to_val(chunk[1]);
			vmask = 0x0F;
		}
	}
	else {
		return false;
	}
	return this->addNode(val, node_type, vmask);
}

bool sig_ma::PckrSign::loadBytes(const BYTE* content, size_t contentSize)
{
	if (this->length() != 0) return false; //already initialized
	for (size_t i = 0; i < contentSize; i++) {
		BYTE val = content[i];
		if (!this->addNode(val, IMM, 0xFF)) break;
	}
	if ((length() == 0)
		|| (contentSize && length() < contentSize))
	{
		return false;
	}
	return true;
}

bool sig_ma::PckrSign::loadByteStr(const std::string& content, size_t expectedSize)
{
	if (this->length() != 0) return false; //already initialized

	std::stringstream input(content);

	// parse all the nodes one by one
	while (!input.eof()) {
		if (expectedSize && (this->length() == expectedSize)) break;

		// parse all chunks from the line
		char chunk[3] = { 0, 0, 0 };
		input >> chunk[0];
		input >> chunk[1];
		if (!parseSigNode(chunk)) break;
	}
	if ((length() == 0)
		|| (expectedSize && length() < expectedSize))
	{
		return false;
	}
	return true;
}

inline std::string to_hex(const uint8_t val, int width = 2)
{
	std::stringstream ss;
	ss << std::setw(width) << std::setfill('0') << std::hex << (unsigned int)(val);
	return ss.str();
}

bool sig_ma::PckrSign::addNode(uint8_t val, sig_type vtype, uint8_t vmask)
{
	nodes.push_back(SigNode(val, vtype, vmask));

	const size_t kMaxPreview = 255;
	if (nodes.size() < kMaxPreview) {
		// add to the content preview:
		if (vtype == IMM) {
			signContent += to_hex(val) + " ";
		}
		else if (vtype == PARTIAL) {
			if (vmask == 0xF0) {
				signContent += to_hex((val >> 4), 1) + "? ";
			} else {
				signContent += "?" + to_hex(val, 1) + " ";
			}
		}
		else if (vtype == WILDC) {
			signContent += "?? ";
		}
	}
	else if (nodes.size() == kMaxPreview) {
		signContent += "...";
	}
	return true;
}
