#ifndef __FLOODROUTING_H__
#define __FLOODROUTING_H__
#include "node.h"
#include "routing.h"

/** Flood routing protocol.

    Simple flood routing protocol. This implementation does assume that keeping
	state for all the nodes in the network is not a problem.
*/
class FloodRouting : public Routing {
	Module_Class_Members(FloodRouting, Routing, 0);

	public:
		virtual void initialize();
		virtual void finish();

		virtual void rx(Packet *msg);
		virtual void tx(Packet *msg);
		virtual void txDone(Packet *msg);
};

#endif

