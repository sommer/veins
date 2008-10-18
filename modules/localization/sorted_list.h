#ifndef SORTED_LIST
#define SORTED_LIST


#include "omnetpp.h"


class SortedList:public cLinkedList {
	bool(*before) (void *a, void *b);

      public:
	void order(bool(*before) (void *a, void *b));
	void insert(void *itm);
};


#endif
