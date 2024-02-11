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
