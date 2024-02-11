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

	template <class Element>
	class ByteMap
	{
	public:
		ByteMap(size_t maxElements, size_t _startIndx = 0)
			: maxCount(maxElements), startIndx(_startIndx), filledCount(0)
		{
			this->list = (Element*) ::calloc(maxCount, sizeof(Element));
			if (!this->list) {
				std::cerr << "Allocating ByteMap failed!\n";
			}
		}

		~ByteMap()
		{
			if (this->list) {
				::free(this->list);
			}
		}

		size_t size()
		{
			return filledCount;
		}

		size_t maxSize()
		{
			return maxCount;
		}

		size_t start()
		{
			return startIndx;
		}

		bool put(size_t indx, const Element &el)
		{
			if (!_isIndxValid(indx)) {
				std::cerr << __FUNCTION__ << ": Invalid index:" << std::hex << indx << std::endl;
				return false;
			}
			const size_t pos = indx - startIndx;
			list[pos] = el;
			this->filledCount++;
			return true;
		}

		bool get(size_t indx, Element &el)
		{
			if (!_isIndxValid(indx)) {
				std::cerr << __FUNCTION__ << ": Invalid index:" << std::hex << indx << std::endl;
				return false;
			}
			const size_t pos = indx - startIndx;
			el = list[pos];
			return true;
		}

		bool erase(size_t indx)
		{
			if (!_isIndxValid(indx)) {
				std::cerr << __FUNCTION__ << ": Invalid index:" << std::hex << indx << std::endl;
				return false;
			}
			const size_t pos = indx - startIndx;
			list[pos] = NULL;
			this->filledCount--;
			return true;
		}

	protected:

		bool _isIndxValid(size_t indx)
		{
			const size_t endIndx = startIndx + maxCount;
			if (indx >= startIndx && indx < endIndx) {
				return true;
			}
			return false;
		}

		size_t filledCount;
		size_t maxCount;
		size_t startIndx;
		Element* list;
	};
	
	//---

	class Node
	{
	public:
		Node()
			: level(0), val(0), mask(MASK_IMM),
			wildcard(nullptr), immediates(0x100), partials(0x100),
			sign(nullptr)
		{
		}

		Node(BYTE _val, size_t _level, BYTE _mask)
			: val(_val), level(_level), mask(_mask),
			wildcard(nullptr), immediates(0x100), partials(0x100),
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
			if (wildcard) delete wildcard;
			wildcard = nullptr;
			if (sign) delete sign;
			sign = nullptr;
		}
		
		bool isEnd()
		{
			return (!immediates.size() && !partials.size() && !wildcard) ? true : false;
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

		Node* _findInChildren(ByteMap<Node*>& children, BYTE _val)
		{
			if (!children.size()) {
				return nullptr;
			}
			Node* next = nullptr;
			if (!children.get(_val, next)) {
				std::cerr << __FUNCTION__ << " Could not get the node at: " << std::hex << _val << std::endl;
			}
			return next;
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

		void _deleteChildren(ByteMap<Node*>& children)
		{
			size_t startIndx = children.start();
			size_t endIndx = startIndx + children.maxSize();
			for (size_t i = startIndx; i < endIndx; i++) {
				Node* next = nullptr;
				if (children.get(i, next)) {
					delete next;
					children.erase(i);
				}
				else {
					std::cerr << __FUNCTION__ << " Could not get the node at: " << std::hex << i << std::endl;
				}
			}
		}

		Signature* sign;
		BYTE val;
		BYTE mask;
		size_t level;
		ByteMap<Node*> immediates;
		ByteMap<Node*> partials;
		Node* wildcard;
	};

}; // namespace sig_finder
