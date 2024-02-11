#include "Node.h"

using namespace sig_finder;

Node* Node::getNode(BYTE _val, BYTE _mask)
{
	BYTE maskedVal = _val & _mask;
	if (_mask == MASK_IMM) {
		return _findInChildren(immediates, maskedVal);
	}
	else if (_mask == MASK_PARTIAL1 || _mask == MASK_PARTIAL2) {
		return _findInChildren(partials, maskedVal);
	}
	else if (_mask == MASK_WILDCARD) {
		return _findInChildren(wildcards, maskedVal);
	}
	return nullptr;
}

Node* Node::addNext(BYTE _val, BYTE _mask)
{
	Node* nextN = getNode(_val, _mask);
	if (nextN) {
		return nextN;
	}

	BYTE maskedVal = _val & _mask;
	nextN = new Node(_val, this->level + 1, _mask);
	if (_mask == MASK_IMM) {
		immediates[maskedVal] = nextN;
	}
	else if (_mask == MASK_PARTIAL1 || _mask == MASK_PARTIAL2) {
		partials[maskedVal] = nextN;
	}
	else if (_mask == MASK_WILDCARD) {
		wildcards[maskedVal] = nextN;
	}
	else {
		delete nextN;
		std::cout << "Invalid mask supplied for value: " << std::hex << (unsigned int)_val << " Mask:" << (unsigned int)_mask << "\n";
		return nullptr; // invalid mask
	}
	return nextN;
}

bool Node::addPattern(const char* _name, const BYTE* pattern, size_t pattern_size, const BYTE* pattern_mask)
{
	if (!pattern || !pattern_size) {
		return false;
	}
	Node* next = this;
	for (size_t i = 0; i < pattern_size; i++) {
		BYTE mask = (pattern_mask != nullptr) ? pattern_mask[i] : MASK_IMM;
		next = next->addNext(pattern[i], mask);
		if (!next) return false;
	}
	if (!next->sign) {
		next->sign = new Signature(_name, pattern, pattern_size, pattern_mask);
	}
	else {
		next->sign->name = _name;
	}
	return true;
}

size_t Node::getMatching(const BYTE* data, size_t data_size, std::vector<Match>& matches, bool stopOnFirst, bool moveStart)
{
	size_t processed = 0;
	//
	ShortList<Node*> level;
	level.push_back(this);
	ShortList<Node*> level2;

	auto level1_ptr = &level;
	auto level2_ptr = &level2;

	for (size_t i = 0; i < data_size; i++)
	{
		processed = i; // processed bytes
		level2_ptr->clear();
		for (size_t k = 0; k < level1_ptr->size(); k++) {
			Node* curr = level1_ptr->at(k);
			if (curr->isSign()) {
				size_t match_start = i - curr->sign->size();
				Match m(match_start, curr->sign);
				matches.push_back(m);
				if (stopOnFirst) {
					return match_start;
				}
			}
			_followAllMasked(level2_ptr, curr, data[i]);
			if (moveStart) {
				if (curr != this) {
					// the current value may also be a beginning of a new pattern:
					_followAllMasked(level2_ptr, this, data[i]);
				}
			}
		}
		if (!level2_ptr->size()) {
			if (moveStart) {
				// if run out of the matches, restart search from the root
				level2_ptr->push_back(this);
			}
			else {
				return processed;
			}
		}
		//swap:
		auto tmp = level1_ptr;
		level1_ptr = level2_ptr;
		level2_ptr = tmp;
	}
	return processed;
}
