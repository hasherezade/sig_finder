#pragma once

template<class Element> class ShortList
{
public:
	ShortList()
		: elCount(0)
	{
	}

	bool push_back(Element n)
	{
		if (elCount >= _countof(list)) {
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
		if (i < _countof(list)) {
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

protected:
	size_t elCount;
	Element list[100];
};