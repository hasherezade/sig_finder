#pragma once

#include "../win_types.h"
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstring>

#include "Signature.h"
#include "Match.h"
#include "ShortList.h"

#define MASK_IMM 0xFF
#define MASK_PARTIAL1 0x0F
#define MASK_PARTIAL2 0xF0
#define MASK_WILDCARD 0

namespace sig_finder {

	class Node
	{
	public:
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
		
		bool isEnd()
		{
			return (!immediates.size() && !partials.size() && !wildcards.size()) ? true : false;
		}

		bool isSign()
		{
			return sign ? true : false;
		}

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

		bool addPattern(const char* _name, const BYTE* pattern, size_t pattern_size, const BYTE* pattern_mask = nullptr);

		bool addTextPattern(const char* pattern1)
		{
			return addPattern(pattern1, (const BYTE*)pattern1, strlen(pattern1));
		}

		bool addPattern(const Signature& sign)
		{
			return addPattern(sign.name.c_str(), sign.pattern, sign.pattern_size, sign.mask);
		}

		size_t getMatching(const BYTE* data, size_t data_size, std::vector<Match>& matches, bool stopOnFirst, bool moveStart = true);

	private:

		Node* getNode(BYTE _val, BYTE _mask);

		Node* addNext(BYTE _val, BYTE _mask);

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

}; // namespace sig_finder
