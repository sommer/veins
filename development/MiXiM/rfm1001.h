#ifndef __RFM1001_H__
#define __RFM1001_H__

#include "radio.h"

class ByteMessage;

/** RFM1001 radio implementation. */
class RFM1001 : public Radio {

	Module_Class_Members(RFM1001, Radio, 0);

	protected:
	/*	void tx(Packet * msg);
		void setTransmit();
		void setListen();
		void setSleep();
		void neighbourStart(unsigned int neigh);
		void neighbourStop(unsigned int neigh);
		void neighbourTx(Packet * msg);
		void rxEnd(Packet * msg);
		void rxHdr(Packet * msg);
		void record_stats();
		void cancelRx();
		void sendRssi();*/
	
	public:
		void initialize();
		void finish();
};


#endif //RADIOH
