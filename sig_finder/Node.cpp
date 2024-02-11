#include "Node.h"

using namespace sig_finder;

Node* Node::getNode(BYTE _val, BYTE _mask)
{
	if (_mask == MASK_IMM) {
		return _findInChildren(immediates, _val);
	}
	else if (_mask == MASK_PARTIAL_R) {
		return _findInChildren(partialsR, (_val & _mask));
	}
	else if (_mask == MASK_PARTIAL_L) {
		return _findInChildren(partialsL, (_val & _mask));
	}
	else if (_mask == MASK_WILDCARD) {
		return wildcard;
	}
	return nullptr;
}

Node* Node::addNext(BYTE _val, BYTE _mask)
{
	Node* nextN = getNode(_val, _mask);
	if (nextN) {
		return nextN;
	}

	nextN = new Node(_val, this->level + 1, _mask);
	if (_mask == MASK_IMM) {
		immediates.put(_val, nextN);
	}
	else if (_mask == MASK_PARTIAL_R) {
		partialsR.put((_val & _mask), nextN);
	}
	else if (_mask == MASK_PARTIAL_L) {
		partialsL.put((_val & _mask), nextN);
	}
	else if (_mask == MASK_WILDCARD) {
		wildcard = nextN;
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
	const size_t MAX_PER_ROUND = moveStart ? 8 : 4; // max count of nodes that can be added to level2 per round

	ShortList<Node*> level1_list(2);
	auto level1 = &level1_list;
	level1->push_back(this);

	ShortList<Node*> level2_list(MAX_PER_ROUND + 1);
	auto level2 = &level2_list;

	for (size_t i = 0; i < data_size; i++, processed++)
	{
		const size_t level2_max = (level1->size() * MAX_PER_ROUND) + 1;
		if (level2_max > level2->maxSize()) {
			if (!level2->resize(level2_max)) {
				std::cerr << "Failed to reallocate!\n";
				return processed;
			}
		}
		level2->clear();
		for (size_t k = 0; k < level1->size(); k++) {
			Node* curr = level1->at(k);
			if (curr->isSign()) {
				size_t match_start = i - curr->sign->size();
				Match m(match_start, curr->sign);
				matches.push_back(m);
				if (stopOnFirst) {
					return match_start;
				}
			}
			_followAllMasked(level2, curr, data[i]); // adds up to 4 nodes to the level2
			if (moveStart) {
				if (curr != this) {
					// the current value may also be a beginning of a new pattern:
					_followAllMasked(level2, this, data[i]); // adds up to 4 nodes to the level2
				}
			}
		}
		if (!level2->size()) {
			if (moveStart) {
				// if run out of the matches, restart search from the root
				level2->push_back(this);
			}
			else {
				return processed;
			}
		}
		//swap:
		auto tmp = level1;
		level1 = level2;
		level2 = tmp;
	}
	return processed;
}
