#ifndef TESTMACLAYER_H_
#define TESTMACLAYER_H_

#include <omnetpp.h>
#include "MacToPhyInterface.h"
#include "MacToPhyControlInfo.h"
#include "TestGlobals.h"
#include "TestPhyLayer.h"
#include "Signal_.h"

class TestMacLayer:public BaseModule, public TestModule {
public:
	MacToPhyInterface* phy;
	TestPhyLayer* testPhy;
	
	int dataOut;
	int dataIn;
	int controlOut;
	int controlIn;
	
	int myIndex;
	
		
	enum {
		TEST_START = 0,
		
		RUN1_TEST_START = 1000,
		RUN1_TEST_ON_RX,
		RUN1_TEST_ON_SLEEP,
		
		RUN2_TEST_START = 2000,
		RUN2_SWITCH_TO_TX,
		RUN2_DURING_SENDING,
		
		RUN3_TEST_START = 3000,
		RUN3_TEST_ON_TX,
		RUN3_TEST_RECEIVE,
		RUN3_TEST_ON_DECIDER1,
		RUN3_TEST_ON_DECIDER2,
		RUN3_TEST_ON_DECIDER3
	};
public:
	//---Omnetpp parts-------------------------------
	virtual void initialize(int stage);	
	
	virtual void handleMessage(cMessage* msg) {
		announceMessage(msg);
		delete msg;
	}
	
	virtual ~TestMacLayer() {
		finalize();
	}
		
	//---Testhandling and test redirection-----------
	/**
	 * This method is called when a message arrives which ahs been awaited
	 * with the "waitForMessage()"-method.
	 * This method should be implemented by every subclass which wants
	 * to eb able to handle asserted messages.
	 */
	virtual void onAssertedMessage(int state, const cMessage* msg);
	
	/**
	 * Redirects test handling to the "testRunx()"-methods
	 * dependend on the current run.
	 */
	void runTests(int state, const cMessage* msg = 0);
	
	/**
	 * Testhandling for run 1:
	 * - check switchRadio() method of MacToPhyInterface
	 * - check sending on not TX mode
	 */
	void testRun1(int stage, const cMessage* msg = 0);
	/**
	 * Testhandling for run 2:
	 * - check sending on already sending
	 */
	void testRun2(int stage, const cMessage* msg = 0);	
	/**
	 * Testhandling for run 3:
	 * - check valid sending of a packet to 3 recipients
	 */
	void testRun3(int stage, const cMessage* msg = 0);
	
	//---run 1 tests------------------------------
	void testGetChannelState();
	void testSwitchRadio(int stage);
	void testChannelSense();
	void testSendingOnNotTX();
	
	//---run 2 tests------------------------------
	
	//---run 3 tests------------------------------
	void testChannelInfo(int stage);
	void testSending1(int stage, const cMessage* lastMsg = 0);
	
	//---utilities--------------------------------
	void continueIn(simtime_t time, int nextStage);
	void waitForTX(int nextStage);
	void sendDown(MacPkt* pkt);	
	MacPkt* createMacPkt(simtime_t length);
};

#endif /*TESTMACLAYER_H_*/
