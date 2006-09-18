#include "neighbourrouting.h"

void NeighbourRouting::addNeighbour(Packet *msg)
{
	int idx = msg->from;
	for (list<NeighbourInfo*>::const_iterator iter = neighbours->begin(); iter!=neighbours->end(); iter++)
	{
		NeighbourInfo *ni = *iter;
		if (ni->idx == idx)
			return;
	}
	NeighbourInfo *ni = (NeighbourInfo*)malloc(sizeof(NeighbourInfo));
	ni->idx = idx;
	neighbours->push_back(ni);
	printf(PRINT_ROUTING,"new neighbour %d",idx);
}

void NeighbourRouting::initialize()
{
	Routing::initialize();
	neighbours = new list<NeighbourInfo*>();
}

NeighbourRouting::~NeighbourRouting()
{
	for (list<NeighbourInfo*>::const_iterator iter = neighbours->begin(); iter!=neighbours->end(); iter++)
		free(*iter);
	delete neighbours;
}

