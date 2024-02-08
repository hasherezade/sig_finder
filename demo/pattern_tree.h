#pragma once

#include <Windows.h>
#include <iostream>
#include <string>
#include <set>
#include <map>
#include <vector>

namespace pattern_tree {

	class Signature
	{
	public:
		Signature(std::string _name, const BYTE* _pattern, size_t _pattern_size)
			: name(_name), pattern(nullptr), pattern_size(0)
		{
			this->pattern = (BYTE*)::calloc(_pattern_size, 1);
			if (!this->pattern) return;

			::memcpy(this->pattern, _pattern, _pattern_size);
			this->pattern_size = _pattern_size;
		}

		size_t pattern_size;
		BYTE* pattern;
		std::string name;
	};

	class Match
	{
	public:
		Match()
			: offset(-1), sign(nullptr)
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

	template<class Element> class ShortList
	{
	public:
		ShortList()
			: elMax(100), elCount(0)
		{
		}

		bool push_back(Element n)
		{
			if (elCount >= elMax) return false;
			list[elCount] = n;
			elCount++;
			return true;
		}

		Element at(size_t i)
		{
			if (i < elCount) {
				return list[i];
			}
			return nullptr;
		}

		void clear()
		{
			elCount = 0;
		}

		size_t size()
		{
			return elCount;
		}

	protected:
		const size_t elMax;
		size_t elCount;
		Element list[100];
	};

	class Node
	{
	public:

		static bool addPattern(Node* rootN, const char* _name, const BYTE* pattern, size_t pattern_size)
		{
			if (!rootN || !pattern) return false;

			Node* next = rootN;
			for (size_t i = 0; i < pattern_size; i++) {
				next = next->addNext(pattern[i]);
				if (!next) return false;
			}
			next->sign = new Signature(_name, pattern, pattern_size);
			return true;
		}

		//---

		Node()
			: level(0), val(0),
			sign(nullptr)
		{
		}

		Node(BYTE _val, size_t _level)
			: val(_val), level(_level),
			sign(nullptr)
		{
		}

		~Node()
		{
			for (auto itr = immediates.begin(); itr != immediates.end(); ++itr) {
				Node* next = itr->second;
				delete next;
			}
			immediates.clear();
			if (sign) {
				delete sign;
			}
		}

		Node* getNode(BYTE _val)
		{
			auto found = immediates.find(_val);
			if (found != immediates.end()) {
				return found->second;
			}
			return nullptr;
		}

		Node* addNext(BYTE _val)
		{
			Node* nextN = getNode(_val);
			if (!nextN) {
				nextN = new Node(_val, this->level + 1);
				immediates[_val] = nextN;
			}
			return nextN;
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

		Match getMatching(const BYTE* data, size_t data_size)
		{
			Match empty;
			//
			ShortList<Node*> level;
			level.push_back(this);
			ShortList<Node*> level2;

			auto level1_ptr = &level;
			auto level2_ptr = &level2;

			for (size_t i = 0; i < data_size; i++)
			{
				level2_ptr->clear();
				for (size_t k = 0; k < level1_ptr->size(); k++) {
					Node * curr = level1_ptr->at(k);
					if (curr->iSign()) {
						const size_t match_start = i - curr->sign->pattern_size;
						return Match(match_start, curr->sign);
					}
					if (curr->isEnd()) return empty;

					curr = curr->getNode(data[i]);
					if (curr) {
						level2_ptr->push_back(curr);
					}
				}
				if (!level2_ptr->size()) return empty;
				//swap:
				auto tmp = level1_ptr;
				level1_ptr = level2_ptr;
				level2_ptr = tmp;
			}
			return empty;
		}

		bool isEnd()
		{
			return this->immediates.size() ? false : true;
		}

		bool iSign()
		{
			return sign ? true : false;
		}

	protected:
		Signature* sign;
		BYTE val;
		size_t level;
		std::map<BYTE, Node*> immediates;
	};

}; //namespace pattern_tree
