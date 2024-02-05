/* 
* Copyright (c) 2013 hasherezade
*/

#include "SigTree.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

//--------------------------------------

#include <ctype.h>

namespace util {

    std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
    {
        str.erase(0, str.find_first_not_of(chars));
        return str;
    }

    std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
    {
        str.erase(str.find_last_not_of(chars) + 1);
        return str;
    }

    std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
    {
        return ltrim(rtrim(str, chars), chars);
    }
}

//--------------------------------------

using namespace sig_ma;

void SigTree::clear()
{
	nodeToSign.clear();

	std::set<PckrSign*>::iterator sigItr;
	for (sigItr = this->all_signatures.begin(); sigItr != this->all_signatures.end(); ++sigItr ) {
		PckrSign* sign = (*sigItr);
		delete sign;
	}
	this->all_signatures.clear();
	signaturesVec.clear();
}

bool SigTree::addPckrSign(PckrSign *sign)
{
	if (!sign) return false;

	SigNode* currNode = &root;

	for (int indx = 0; indx < sign->nodes.size(); indx++) {
		SigNode* nextNode = currNode->putChild(sign->nodes[indx]);
		if (!nextNode) {
			// Can't add the signature
			return false;
		}
		currNode = nextNode;
	}

	if (!nodeToSign[currNode]) {
		// add new
		nodeToSign[currNode] = sign;
		insertPckrSign(sign);
		return true;
	}
	// already exists
	return true;
}

void SigTree::insertPckrSign(PckrSign* sign)
{
	if (!sign) return;
	size_t signCount = all_signatures.size();
	all_signatures.insert(sign);

	if (all_signatures.size() > signCount) {
		signaturesVec.push_back(sign);
	}

	size_t len = sign->length();
	if (this->min_siglen == 0 || this->min_siglen > len) 
		this->min_siglen = len;
	
	if (this->max_siglen == 0 || this->max_siglen < len) 
		this->max_siglen = len;
}

void SigTree::_storeFound(SigNode *nextC, std::vector<SigNode*>& level2, matched &matchedSet)
{
	if (!nextC) {
		return;
	}
	PckrSign *sig = this->nodeToSign[nextC];
	if (sig) {
		matchedSet.signs.insert(sig);
	}
	level2.push_back(nextC);
}

matched SigTree::getMatching(const uint8_t *buf, const size_t buf_len, bool skipNOPs)
{
	matched matchedSet;
	matchedSet.match_offset = 0;

	if (!buf) return matchedSet; // Empty

	std::vector<SigNode*> level;
	level.push_back(&root);

	long checked = 0;
	long skipped = 0;
	for (size_t indx = 0; indx < buf_len; indx++) {
		const uint8_t bufChar = buf[indx];

		std::vector<SigNode*> level2;
		
		for (std::vector<SigNode*>::const_iterator lvlI = level.begin(); lvlI != level.end(); ++lvlI) {
			const SigNode* currNode = (*lvlI);
			if (!currNode) continue;
			
			// allow for alternate sig search paths: with wildcards AND with exact matches
			_storeFound(currNode->getChild(bufChar), level2, matchedSet);
			_storeFound(currNode->getPartial(buf[indx] & 0xF0), level2, matchedSet);
			_storeFound(currNode->getPartial(buf[indx] & 0x0F), level2, matchedSet);
			_storeFound(currNode->getWildc(), level2, matchedSet);
		}
		//-----
		if (level2.size() == 0) {
			if (buf[indx] == OP_NOP && skipNOPs) { //skip NOPs
				if (checked == 0) {
					matchedSet.match_offset++;
				}
				skipped++;
				continue;
			}
			break;
		}
		level = level2;
		checked++;
	}
	return matchedSet;
}

bool SigTree::parseSigNode(PckrSign &sign, char chunk[3])
{
	sig_type node_type = sig_type::ROOT;
	unsigned int val = 0;
	uint8_t vmask = 0xFF;
	if (is_hex(chunk[0]) && is_hex(chunk[1])) {
		node_type = IMM;
		val = (hex_char_to_val(chunk[0]) << 4) | (hex_char_to_val(chunk[1]));
	}
	else if (chunk[0] == WILD_ONE && chunk[1] == WILD_ONE ) {
		node_type = WILDC;
		val = chunk[0];
		vmask = 0;
	} else if (chunk[0] == WILD_ONE || chunk[1] == WILD_ONE ) {
		node_type = PARTIAL;
		if (chunk[1] == WILD_ONE && is_hex(chunk[0])) {
			val = hex_char_to_val(chunk[0]) << 4;
			vmask = 0xF0;
		} else if (chunk[0] == WILD_ONE && is_hex(chunk[1])) {
			val = hex_char_to_val(chunk[1]);
			vmask = 0x0F;
		}
	} else {
		return false;
	}
	return sign.addNode(val, node_type, vmask);
}

bool SigTree::loadSignature(const std::string &name, const std::string &content, size_t expectedSize)
{
	PckrSign *sign = new PckrSign(name); // <- new signature created

	std::stringstream input(content);
	while (!input.eof()) {
		if (expectedSize && (sign->length() == expectedSize)) break;

		// parse all chunks from the line
		char chunk[3] = { 0, 0, 0 };
		input >> chunk[0];
		input >> chunk[1];
		if (!parseSigNode(*sign, chunk)) break;
	}
	if ((sign->length() == 0)
		|| (expectedSize && sign->length() < expectedSize))
	{
		delete sign;
		return false;
	}
	return this->addPckrSign(sign);
}

size_t SigTree::loadFromFile(std::ifstream& input)
{
	if (!input.is_open()) return 0;

	sig_type type = ROOT;
	size_t loadedNum = 0;

	while (!input.eof()) {
		std::string line;

		// read signature name
		if (!std::getline(input, line)) break;
		std::string name = util::trim(line);

		// read signature size
		if (!std::getline(input, line)) break;
		int signSize = 0;
		std::stringstream iss1;
		iss1 << std::dec << line;
		iss1 >> signSize; //read the expected size
		if (signSize == 0) continue;

		PckrSign *sign = new PckrSign(name); // <- new signature created

		bool isFin = false;
		// read signature chunks:
		while ( !input.eof() && (sign->length() < signSize) ){

			// parse all chunks from the line
			char chunk[3] = { 0, 0, 0 };
			input >> chunk[0];
			input >> chunk[1];
			if (!parseSigNode(*sign, chunk)) break;
		}
		// check if the signature is valid:
		if (sign->length() == signSize) {
			if (this->addPckrSign(sign)) { // <- new signature stored
				loadedNum++;
				continue; // success
			}
		}
		//failure:
		delete sign;
		sign = NULL;
	}
	return loadedNum;
}
