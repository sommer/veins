#ifndef __NEIGHBOUR_ROUTING_H__
#define __NEIGHBOUR_ROUTING_H__

#include "routing.h"

struct NeighbourInfo {
	int idx;
};

class NeighbourRouting: public Routing
{
	protected:
		list<NeighbourInfo*> *neighbours;
		~NeighbourRouting();
	public:
		Module_Class_Members(NeighbourRouting, Routing, 0) 
		void initialize();
		void addNeighbour(Packet *msg);
};

#endif

