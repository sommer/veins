#ifndef __MAGICSINKROUTING_H__
#define __MAGICSINKROUTING_H__
#include "sinkRoutingBase.h"

/** Sink routing protocol.

    This protocol builds up a spanning tree to the sink. It can only route to
	the sink (node 0 by default).
*/
class MagicSinkRouting : public SinkRoutingBase {
	Module_Class_Members(MagicSinkRouting, SinkRoutingBase, 0);

	private:
		static bool parametersInitialised;

	public:
		virtual void initialize();
		virtual void finish();
		virtual ~MagicSinkRouting();
	
		virtual void handleMessage(cMessage *msg);
};

#endif 
