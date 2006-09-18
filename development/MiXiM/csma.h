#ifndef __CSMAH__
#define __CSMAH__

#include "mac.h"

class Csma: public Mac {

	// contructor, destructor, module stuff
	Module_Class_Members(Csma, Mac, 0);
	~Csma();

protected:
	/** Current state of the MAC layer. */
	int state;
	/** Pointer to message that the MAC layer is trying to send. */
	Packet * tx_msg;

	/** Change state into idle. */
	void gotoIdle();
	/** Change state delaying before transmission. */
	void gotoDelaying();
	/** Change state into carier sense. */
	void gotoSensing();
	/** Change state into transmiting. */
	void gotoTransmit();
	/** Handle the end of receiving, whether successful or failed. */
	void rxEnd();

	virtual void initialize();
	virtual void finish();
	virtual void timeout();
	virtual void txPacket(Packet * msg);
	virtual void rxStarted() {}
	virtual void rxFrame(Packet * msg);
	virtual void transmitDone();
	virtual void rxFailed();
	virtual int headerLength();
	
	virtual void endForce(){};
};


#endif // __CSMAH__
