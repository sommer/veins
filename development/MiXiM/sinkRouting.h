#ifndef __SINKROUTING_H__
#define __SINKROUTING_H__
#include "node.h"
#include "sinkRoutingBase.h"

/** Sink routing protocol.

    This protocol builds up a spanning tree to the sink. It can only route to
	the sink (node 0 by default).
*/
class SinkRouting : public SinkRoutingBase {
	Module_Class_Members(SinkRouting, SinkRoutingBase, 0);

	private:
		static bool parametersInitialised;

	public:
		virtual void initialize();
		virtual void finish();
		virtual ~SinkRouting();

		virtual void handleMessage(cMessage *msg);
		
	protected:
		/** Time between broadcast floods. */
		static double floodInterval,
		/** Initial wait, before sending a flood. */
			initWait;

		int serial;
};

#endif 
