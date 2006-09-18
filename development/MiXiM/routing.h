#ifndef __ROUTING_H__
#define __ROUTING_H__

#include "mixim.h"
#include "node.h"
#include <map>

class Routing : public MiximSoftwareModule {
	Module_Class_Members(Routing, MiximSoftwareModule, 0);

	private:
		static bool parametersInitialised;

	public:
		virtual void initialize();
		virtual void finish();
		virtual void addToQueue(cMessage *msg);
		void sendToMac(Packet *);
		void sendToApp(Packet* msg);
		virtual ~Routing();

		virtual void rx(Packet *msg) = 0;
		virtual void tx(Packet *msg) = 0;
		virtual void txDone(Packet *msg) = 0;
		virtual void txFailed(Packet *msg);
		virtual void txNoAck(Packet *msg);
		virtual void txContendLose(Packet *msg);
		virtual void txRetrySkipped(Packet *msg);
		virtual void txSkipped(Packet *msg);
		virtual void forceGrant(bool success);
	
	protected:
		static int maxQueueLength, preferTXThreshold;
	
		int stat_tx, stat_rx, stat_tx_drop, stat_rx_dup;

		int mySerial;
	
		/** Simulate processor cycles use.
		    @param cycles The number of cycles to be used in simulation.

		    This routine can be used to simulate that code needs cpu-cycles to
		    execute. The cycles used will then be taken into account in
		    calculating the amount of power used by the processor.
	
		    Note that this simply calls the eatCycles routine in @b Node.
		*/
		virtual void eatCycles(unsigned cycles) { node->eatCycles(cycles); }
		/** Pointer to the enclosing @b Node. */
		Node* node;

		/** A queue to store messages waiting to b handled by the MAC layer. */
		cQueue *msgQueue;
		/** Boolean to store whether the MAC layer is currently trying to send
		    a packet for us. */
		bool macBusy;

		bool registerSerial(int nodeId, int serial);
		
		struct SerialInfo {
			int lastSerial;
			int bitmap;
		};
		
		map<int, SerialInfo> serials;
		
		virtual void handleEvent(cMessage *msg);
};

#endif
 
