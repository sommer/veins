#include "sorted_list.h"



void SortedList::order(bool(*before) (void *a, void *b))
{
	this->before = before;
}


void SortedList::insert(void *itm)
{
	void *prev = NULL;

	for (cLinkedList::Iterator iter(*this); !iter.end(); iter++) {
		void *curr = iter();

		if (before(itm, curr)) {
			break;
		}
		prev = curr;
	}
	if (prev == NULL) {
		cLinkedList::insert(itm);
	} else {
		cLinkedList::insertAfter(prev, itm);
	}
}
