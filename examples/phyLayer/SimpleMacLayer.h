#ifndef SIMPLEMACLAYER_H_
#define SIMPLEMACLAYER_H_

#include <omnetpp.h>
#include "MacToPhyInterface.h"
#include "MacToPhyControlInfo.h"
#include "Signal_.h"
#include "MacPkt_m.h"

class SimpleMacLayer:public BaseModule {
protected:
	MacToPhyInterface* phy;
	
	int dataOut;
	int dataIn;
	
	int myIndex;
	int hostCount;
	
	int nextReceiver;
	
	enum {
		TEST_MACPKT = 12121
	};
	
public:
	//---Omnetpp parts-------------------------------
	virtual void initialize(int stage);	
	
	virtual void handleMessage(cMessage* msg);
	
	void handleMacPkt(MacPkt* pkt);
	void handleTXOver();
	
	void broadCastPacket();	
	
	void sendDown(MacPkt* pkt);	
	MacPkt* createMacPkt(simtime_t length);
	
	void log(std::string msg);
};

#endif /*TESTMACLAYER_H_*/
