#ifndef __PATHLOSSRADIOPROPAGATIONMODEL_H__
#define __PATHLOSSRADIOPROPAGATIONMODEL_H__

#include "propagationModel.h"
#include "node.h"

class NodeContainer; // hidden in node_n.cc

/** Unit sphere propagation model. */
class PathLossRadioPropagationModel: public PropagationModel {
	Module_Class_Members(PathLossRadioPropagationModel, PropagationModel, 0);

	public:
		virtual void initialize(int);
		virtual void finish();
		void handleMessage(cMessage *msg);
		int numInitStages() const { return 2; }
		virtual ~PathLossRadioPropagationModel();

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

		/** Propagation model parameter, calculated from radio characteristics. */
		static double beta;
		//FIXME: use per message TX power!
		static double d0power;
	
		static bool parametersInitialised;
};

#endif

