#ifndef __CC1K_H__
#define __CC1K_H__

#include "radio.h"

/** Class that simulates a CC1000 radio. */
class CC1000: public Radio {
	Module_Class_Members(CC1000, Radio, 0);

	protected:
	/*	virtual void tx(Packet * msg);
		virtual void setTransmit();
		virtual void setListen();
		virtual void setSleep();
		virtual void neighbourStart(unsigned int neigh);
		virtual void neighbourStop(unsigned int neigh);
		virtual void neighbourTx(Packet * msg);
		virtual void rxEnd(Packet * msg);
		virtual void rxHdr(Packet * msg);
		virtual void record_stats();
		virtual void cancelRx();
		virtual void sendRssi();*/

	public:
		void initialize();
		void finish();
};

#endif

