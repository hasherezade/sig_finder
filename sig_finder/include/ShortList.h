#pragma once

template<class Element> class ShortList
{
public:
	ShortList()
		: elCount(0),
		maxCount(sizeof(list) / sizeof(Element))
	{
	}

	bool push_back(Element n)
	{
		if (elCount >= maxCount) {
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

protected:

	const size_t maxCount;
	size_t elCount;
	Element list[100];
};
