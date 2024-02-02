/*
 * Copyright (c) 2013 hasherezade
*/

#pragma once

#include <stdio.h>
#include <set>
#include <vector>
#include <string>
#include <string.h>
#include <stdlib.h>
#include "../win_types.h"

#define OP_NOP 0x90


namespace sig_ma {
//------------------
enum sig_wildc {
	WILD_NONE = 0,
	WILD_ONE = '?'
};

enum sig_type {
    ROOT = 0,
    IMM,
    PARTIAL,
    WILDC,
    MATCH
};

bool inline is_hex(char c)
{
	if ((c >= '0' && c <='9') || (c >='a' && c <='f') || (c >='A' && c <='F')) return true;
	return false;
};

char inline hex_char_to_val(char c)
{
	if (c >= '0' && c <='9') {
		return c - '0';
	}
	if (c >='a' && c <='f') {
		return c - 'a' + 10;
	}
	if (c >='A' && c <='F') {
		return c - 'A' + 10;
	}
	return 0;
};


class SigNode
{
public:
	struct sig_compare {
		bool operator()(const SigNode* el1, const SigNode* el2 ) const;
	};
	
	SigNode(uint8_t val, sig_type type = IMM, uint8_t vmask = 0xFF);
	~SigNode();

	SigNode* getWildc() const;
	SigNode* getChild(uint8_t val) const;
	SigNode* getPartial(uint8_t val) const;

	SigNode* putChild(uint8_t val, uint8_t vmask);
	SigNode* putWildcard(uint8_t val);

	bool operator==(const SigNode &el) const { return el.v == this->v; }
	bool operator!=(const SigNode &el) const { return el.v != this->v; }
	bool operator<(const SigNode &el) const { return el.v < this->v; }

private:
	SigNode* _insertSigNode(std::set<SigNode*, SigNode::sig_compare> &nodesSet, uint8_t val, uint8_t vmask, sig_type vtype);
	void _clearNodesSet(std::set<SigNode*, sig_compare> &nodesSet);
	
	uint8_t v;
	uint8_t vmask;
	sig_type vtype;
	std::set<SigNode*, sig_compare> immediates;
	std::set<SigNode*, sig_compare> partials;
	std::set<SigNode*, sig_compare> wildcards;

friend class SigTree;
};

}; /* namespace sig_ma */
