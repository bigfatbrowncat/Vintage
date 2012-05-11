#ifndef CHAIN_H_
#define CHAIN_H_

#include "globals.h"

template<typename T> struct chain
{
	char* id_name;
	T value;

	chain<T>* next;

	bool findId(char* name, T& addr)
	{
		chain<T>* cur = this;
		while (cur != NULL)
		{
			if (areEqual(name, cur->id_name))
			{
				addr = cur->value;
				return true;
			}
			cur = cur->next;
		}
		return false;
	}

	bool addId(char* name, T& addr)
	{
		chain<T>* cur = this;
		chain<T>* prev = NULL;
		while (cur != NULL)
		{
			prev = cur;
			if (areEqual(name, cur->id_name))
			{
				return false;
			}
			cur = cur->next;
		}
		if (prev != NULL)
		{
			prev->next = new chain;

			// Calculating length
			int l = 0;
			char *c21 = name;
			while (*c21 != 0) { c21 ++; l ++; }

			prev->next->id_name = new char[l + 1];
			char *c1 = name, *c2 = prev->next->id_name;
			do
			{
				*c2 = *c1;
				c1++; c2++;
			}
			while (*c1 != 0);
			*c2 = 0;

			prev->next->value = addr;
			prev->next->next = NULL;
			return true;
		}
		return false;
	}

	void freeIds()
	{
		chain<T>* cur = this;
		while (cur != NULL)
		{
			chain *tokill = cur->next;
			delete[] cur->id_name;
			delete cur;
			cur = tokill;
		}
	}

};

#endif /* CHAIN_H_ */
