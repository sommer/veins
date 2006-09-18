#ifndef __NOROUTING_H__
#define __NOROUTING_H__

#include "routing.h"

/** Class that implements a dummy routing protocol.

    This class can be used when no routing protocol is desired. It does
    implement a queue, so that the application doesn't have to worry about the
    MAC layer being busy.
*/
class NoRouting : public Routing {
	Module_Class_Members(NoRouting, Routing, 0);

	private:
		static bool parametersInitialised;

	public:
		virtual void initialize();
		virtual void finish();

		virtual void rx(Packet *packet);
		virtual void tx(Packet *packet);
		virtual void txDone(Packet *packet);
		virtual void txNoAck(Packet *packet);
		virtual void txSkipped(Packet *packet);
	
	protected:
		Packet *selectPacket(Packet *packet);
		static int max_retries;
		static bool gmacLowLatency;
};

#endif

