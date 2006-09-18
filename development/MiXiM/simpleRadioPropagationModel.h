#ifndef __SIMPLERADIOPROPAGATIONMODEL_H__
#define __SIMPLERADIOPROPAGATIONMODEL_H__

#include "propagationModel.h"
#include "node.h"

class NodeContainer; // hidden in node_n.cc

/** Unit sphere propagation model. */
class SimpleRadioPropagationModel: public PropagationModel {
	Module_Class_Members(SimpleRadioPropagationModel, PropagationModel, 0);

	public:
		virtual void initialize();
		virtual void finish();
		void handleMessage(cMessage *msg);
	
	private:
		/** Send a @c TX_START message to all nodes within range. */
		void start_tx(cModule *senderModule);
		/** Send a @c TX_STOP message to all nodes within range. */
		void stop_tx(cModule *senderModule);
		/** Send a @b Packet to all nodes in range.
		    @param msg The @b Packet to send.
		*/
		void distributePacket(Packet *msg, cModule *senderModule);
		
		/** Pointer to the network container-module. */
		cModule* network;
};

#endif

