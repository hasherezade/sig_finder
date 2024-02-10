#pragma once

#include "../win_types.h"
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstring>

#include "ShortList.h"

#define MASK_IMM 0xFF
#define MASK_PARTIAL1 0x0F
#define MASK_PARTIAL2 0xF0
#define MASK_WILDCARD 0

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

	class Match
	{
	public:
		Match()
			: offset(0), sign(nullptr)
		{
		}

		Match(size_t _offset, Signature* _sign)
			: offset(_offset), sign(_sign)
		{
		}

		Match(const Match& _match) // copy constructor
		{
			offset = _match.offset;
			sign = _match.sign;
		}

		size_t offset;
		Signature* sign;
	};

	class Node
	{
	public:
		size_t addPatterns(const std::vector<Signature*>& signatures)
		{
			size_t loaded = 0;
			for (auto itr = signatures.begin(); itr != signatures.end(); ++itr) {
				const Signature* sign = *itr;
				if (!sign) continue;
				if (this->addPattern(*sign)) {
					loaded++;
				}
			}
			return loaded;
		}

		bool addPattern(const char* _name, const BYTE* pattern, size_t pattern_size, const BYTE* pattern_mask=nullptr)
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
			} else {
				next->sign->name = _name;
			}
			return true;
		}

		bool addTextPattern(const char* pattern1)
		{
			return addPattern(pattern1, (const BYTE*)pattern1, strlen(pattern1));
		}

		bool addPattern(const Signature& sign)
		{
			return addPattern(sign.name.c_str(), sign.pattern, sign.pattern_size, sign.mask);
		}

		//---

		Node()
			: level(0), val(0), mask(MASK_IMM),
			sign(nullptr)
		{
		}

		Node(BYTE _val, size_t _level, BYTE _mask)
			: val(_val), level(_level), mask(_mask),
			sign(nullptr)
		{
		}

		~Node()
		{
			clear();
		}
		
		void clear()
		{
			_deleteChildren(immediates);
			_deleteChildren(partials);
			_deleteChildren(wildcards);
			if (sign) {
				delete sign;
			}
		}

		void print()
		{
			std::cout << std::hex << (unsigned int)val << " [" << level << "]" << " [" << immediates.size() << "]";
			if (!immediates.size()) {
				printf("\n");
				return;
			}
			for (auto itr = immediates.begin(); itr != immediates.end(); ++itr) {
				itr->second->print();
			}
		}

		size_t getMatching(const BYTE* data, size_t data_size, std::vector<Match> &matches, bool stopOnFirst, bool moveStart = true)
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
					} else {
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

		bool isEnd()
		{
			return (!immediates.size() && !partials.size() && !wildcards.size()) ? true : false;
		}

		bool isSign()
		{
			return sign ? true : false;
		}

	private:

		Node* getNode(BYTE _val, BYTE _mask)
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

		Node* addNext(BYTE _val, BYTE _mask)
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

		Node* _findInChildren(std::map<BYTE, Node*>& children, BYTE _val)
		{
			if (!children.size()) {
				return nullptr;
			}
			auto found = children.find(_val);
			if (found != children.end()) {
				return found->second;
			}
			return nullptr;
		}

		bool _followMasked(ShortList<Node*>* level2_ptr, Node* curr, BYTE val, BYTE mask)
		{
			Node* next = curr->getNode(val, mask);
			if (!next) {
				return false;
			}
			return level2_ptr->push_back(next);
		}

		void _followAllMasked(ShortList<Node*>* level2_ptr, Node* node, BYTE val)
		{
			_followMasked(level2_ptr, node, val, MASK_IMM);
			_followMasked(level2_ptr, node, val, MASK_PARTIAL1);
			_followMasked(level2_ptr, node, val, MASK_PARTIAL2);
			_followMasked(level2_ptr, node, val, MASK_WILDCARD);
		}

		void _deleteChildren(std::map<BYTE, Node*>& children)
		{
			for (auto itr = children.begin(); itr != children.end(); ++itr) {
				Node* next = itr->second;
				delete next;
			}
			children.clear();
		}

		Signature* sign;
		BYTE val;
		BYTE mask;
		size_t level;
		std::map<BYTE, Node*> immediates;
		std::map<BYTE, Node*> partials;
		std::map<BYTE, Node*> wildcards;
	};

	inline size_t find_all_matches(Node& rootN, const BYTE* loadedData, size_t loadedSize, std::vector<Match>& allMatches)
	{
		if (!loadedData || !loadedSize) {
			return 0;
		}
		rootN.getMatching(loadedData, loadedSize, allMatches, false);
		return allMatches.size();
	}

	inline Match find_first_match(Node& rootN, const BYTE* loadedData, size_t loadedSize)
	{
		Match empty;
		if (!loadedData || !loadedSize) {
			return empty;
		}
		std::vector<Match> allMatches;
		rootN.getMatching(loadedData, loadedSize, allMatches, true);
		if (allMatches.size()) {
			auto itr = allMatches.begin();
			return *itr;
		}
		return empty;
	}

}; //namespace sig_finder
