#pragma once
#include <cstdlib>

#define INVALID_INDEX (-1)

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
		if (!this->list) { //should not be NULL
			return false;
		}
		this->list = (Element*)::realloc(this->list, newElementCount * sizeof(Element));
		if (this->list) {
			this->maxCount = newElementCount;
			return true;
		}
		std::cerr << "Failed to reallocate for the element count: " << std::dec << newElementCount << std::endl;
		return  false;
	}

	bool push_back(Element n)
	{
		if (elCount >= maxCount) {
			std::cerr << "Short list overflowed, starting to drop elements:" << std::dec << maxCount << std::endl;
			return false;
		}
		size_t indx = _getItemIndex(n);
		if (indx == INVALID_INDEX) {
			return this->_addItemSorted(n);
		}
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
	bool _addItemSorted(Element it)
	{
		size_t itIndx = _getItemIndex(it);
		size_t indx = _findFirstGreater(it, itIndx);
		if (indx == INVALID_INDEX) {
			list[this->elCount++] = it;
			return true;
		}
		if (!_shiftItemsRight(indx)) {
			return false;
		}
		list[indx] = it;
		return true;
	}

	size_t _findFirstGreater(Element it, size_t startIndx = 0)
	{
		if (elCount == 0) {
			return INVALID_INDEX;
		}
		if (startIndx == INVALID_INDEX) {
			startIndx = 0;
		}
		for (size_t i = startIndx; i < elCount; i++)
		{
			if (list[i] > it) {
				return i;
			}
		}
		return elCount;
	}

	bool _shiftItemsRight(size_t startIndx)
	{
		if (startIndx == INVALID_INDEX) {
			return false;
		}
		if (elCount >= maxCount) {
			return false;
		}
		for (size_t i = elCount; i > startIndx; i--) {
			list[i] = list[i - 1];
		}
		elCount++;
		return true;
	}

	bool _shiftItemsLeft(int startIndx)
	{
		if (startIndx == INVALID_INDEX) {
			return false;
		}
		for (size_t i = startIndx + 1; i < elCount; i++) {
			list[i - 1] = list[i];
		}
		list[ItemCount - 1] = 0;
		list--;
		return true;
	}

	size_t _getItemIndex(Element it)
	{
		if (!list || elCount == 0) {
			return INVALID_INDEX;
		}
		size_t start = 0;
		size_t stop = elCount;
		while (start < stop) {
			size_t mIndx = (start + stop) / 2;
			if (list[mIndx] == it) {
				return mIndx;
			}
			if (list[mIndx] < it) {
				start = mIndx + 1;
			}
			else if (list[mIndx] > it) {
				stop = mIndx;
			}
		}
		return INVALID_INDEX;
	}


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
