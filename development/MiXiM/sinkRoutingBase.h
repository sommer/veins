#ifndef __SINKROUTINGBASE_H__
#define __SINKROUTINGBASE_H__
#include "node.h"
#include "routing.h"
#include <vector>

/** Sink routing protocol base.

    This protocol builds up a spanning tree to the sink. It can only route to
	the sink (node 0 by default). If a message is handed to this routing layer
	that names an address that is not the sink as destination, it is simply
	sent to the named address. This is to accomodate local UC and BC.
*/

class SinkRoutingBase : public Routing {
	Module_Class_Members(SinkRoutingBase, Routing, 0);

	private:
		static bool parametersInitialised;

	public:
		virtual void initialize();
		virtual void finish();
		virtual ~SinkRoutingBase();

		virtual void rx(Packet *packet);
		virtual void tx(Packet *packet);
		virtual void txDone(Packet *packet);
		virtual void txNoAck(Packet *packet);
		virtual void txSkipped(Packet *packet);
	
		/** Add a message to the message queue, and process the queue. */
		virtual void runQueue(Packet *msg);
	
		virtual void setLocalTo(Packet *packet);
		
	protected:
		/** Nodes to forward messages to. */
		vector<int> parents;
		/** Number of hops parent is away from sink. */
		int parentHops,
		/** Fixed parent number from list. */
			parent,
		/** Retries for last message. */
			retries;
		/** Node that will be routed to. */
		static int destination,
		/** Maximum number of retries for rerouted messages. */
			max_retries;
		/** Use a fixed parent for sending messages, instead of a random one. */
		static bool fixedParent;
		/** Try to use G-MAC slot information to reduce latency. */
		static bool gmacLowLatency;

		enum MsgType {
			ROUTE_SETUP,
			ROUTE_DATA
		};
		struct Header {
			MsgType type;
			int hopsToSink;
			int serial;
		};
};

#endif 
