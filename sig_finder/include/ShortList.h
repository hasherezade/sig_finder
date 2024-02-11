#pragma once
#include <cstdlib>

template <class Element>
class ShortList
{
public:
	ShortList(size_t maxElements)
		: elCount(0), list(nullptr),
		maxCount(maxElements)
	{
		this->list = (Element*) ::malloc(maxCount * sizeof(Element));
		if (!this->list) {
			std::cerr << "Allocating ShortList failed!\n";
		}
	}
	~ShortList()
	{
		if (this->list) {
			::free(this->list);
		}
	}

	bool resize(size_t newElementCount)
	{
		this->list = (Element*)::realloc(this->list, newElementCount * sizeof(Element));
		if (this->list) {
			this->maxCount = newElementCount;
			return true;
		}
		return  false;
	}

	bool push_back(Element n)
	{
		if (elCount >= maxCount) {
			std::cerr << "Short list overflowed, starting to drop elements:" << maxCount << std::endl;
			return false;
		}
		if (find(n)) {
			return true;
		}
		list[elCount] = n;
		elCount++;
		return true;
	}

	Element at(size_t i)
	{
		if (i < maxCount) {
			return list[i];
		}
		return nullptr;
	}

	Element find(Element& searched)
	{
		for (size_t i = 0; i < elCount; i++) {
			if (list[i] == searched) {
				return list[i];
			}
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

	size_t maxSize()
	{
		return maxCount;
	}

protected:
	size_t maxCount;
	size_t elCount;
	Element* list;
};

//---

template <class Element>
class ShortMap
{
public:
	ShortMap(size_t maxElements, size_t _startIndx = 0)
		: maxCount(maxElements), startIndx(_startIndx), filledCount(0)
	{
		this->list = (Element*) ::calloc(maxCount, sizeof(Element));
		if (!this->list) {
			std::cerr << "Allocating ByteMap failed!\n";
		}
	}

	~ShortMap()
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

	bool put(size_t indx, const Element& el)
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

	bool get(size_t indx, Element& el)
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
