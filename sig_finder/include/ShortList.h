#pragma once
#include <cstdlib>

template <class Element>
class ShortList
{
public:
	ShortList(size_t maxElements)
		: ItemsCount(0), Items(nullptr),
		MaxItemsCount(maxElements)
	{
		this->Items = (Element*) ::malloc(MaxItemsCount * sizeof(Element));
		if (!this->Items) {
			std::cerr << "Allocating ShortList failed!\n";
		}
	}
	~ShortList()
	{
		if (this->Items) {
			::free(this->Items);
		}
	}

	bool resize(size_t newItemsCount)
	{
		if (!this->Items) { // don't try to reallocate the list if it was NULL
			return false;
		}
		this->Items = (Element*)::realloc(this->Items, newItemsCount * sizeof(Element));
		if (this->Items) {
			this->MaxItemsCount = newItemsCount;
			return true;
		}
		std::cerr << "Failed to reallocate the list for: " << std::dec << newItemsCount << " elements!\n";
		return  false;
	}

	bool push_back(Element n)
	{
		if (ItemsCount >= MaxItemsCount) {
			std::cerr << "Short list overflowed, starting to drop elements:" << std::dec << MaxItemsCount << std::endl;
			return false;
		}
		if (find(n)) {
			return true;
		}
		Items[ItemsCount] = n;
		ItemsCount++;
		return true;
	}

	Element at(size_t i)
	{
		if (i < MaxItemsCount) {
			return Items[i];
		}
		return nullptr;
	}

	Element find(Element& searched)
	{
		for (size_t i = 0; i < ItemsCount; i++) {
			if (Items[i] == searched) {
				return Items[i];
			}
		}
		return nullptr;
	}

	void clear()
	{
		ItemsCount = 0;
	}

	size_t size()
	{
		return ItemsCount;
	}

	size_t maxSize()
	{
		return MaxItemsCount;
	}

protected:
	size_t MaxItemsCount;
	size_t ItemsCount;
	Element* Items;
};

//---

template <class Element>
class ShortMap
{
public:
	ShortMap(size_t maxElements, size_t _startIndx = 0)
		: MaxItemsCount(maxElements), StartIndx(_startIndx), ItemsCount(0)
	{
		this->Items = (Element*) ::calloc(MaxItemsCount, sizeof(Element));
		if (!this->Items) {
			std::cerr << "Allocating ByteMap failed!\n";
		}
	}

	~ShortMap()
	{
		if (this->Items) {
			::free(this->Items);
		}
	}

	size_t size()
	{
		return ItemsCount;
	}

	size_t maxSize()
	{
		return MaxItemsCount;
	}

	size_t start()
	{
		return StartIndx;
	}

	bool put(size_t indx, const Element& el)
	{
		if (!_isIndxValid(indx)) {
			std::cerr << __FUNCTION__ << ": Invalid index:" << std::hex << indx << std::endl;
			return false;
		}
		const size_t pos = indx - StartIndx;
		Items[pos] = el;
		this->ItemsCount++;
		return true;
	}

	bool get(size_t indx, Element& el)
	{
		if (!_isIndxValid(indx)) {
			std::cerr << __FUNCTION__ << ": Invalid index:" << std::hex << indx << std::endl;
			return false;
		}
		const size_t pos = indx - StartIndx;
		el = Items[pos];
		return true;
	}

	bool erase(size_t indx)
	{
		if (!_isIndxValid(indx)) {
			std::cerr << __FUNCTION__ << ": Invalid index:" << std::hex << indx << std::endl;
			return false;
		}
		const size_t pos = indx - StartIndx;
		Items[pos] = NULL;
		this->ItemsCount--;
		return true;
	}

protected:

	bool _isIndxValid(size_t indx)
	{
		const size_t endIndx = StartIndx + MaxItemsCount;
		if (indx >= StartIndx && indx < endIndx) {
			return true;
		}
		return false;
	}

	size_t ItemsCount;
	size_t MaxItemsCount;
	size_t StartIndx;
	Element* Items;
};
