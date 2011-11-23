#ifndef TESTMACLAYER_H_
#define TESTMACLAYER_H_

#include <omnetpp.h>
#include "MacToPhyInterface.h"
#include "TestGlobals.h"
#include "TestPhyLayer.h"
#include "Signal_.h"

class MacPkt;

class TestMacLayer:public BaseModule, public TestModule
{
protected:
	MacToPhyInterface* phy;
	TestPhyLayer* testPhy;

	int dataOut;
	int dataIn;
	int controlOut;
	int controlIn;

	int myIndex;

	int run;

protected:


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
	 * Redirects test handling to the "testRunx()"-methods
	 * dependend on the current run.
	 */
	void runTests(int run, int state, const cMessage* msg = 0);

	/**
	 * @brief Test handling for run 1:
	 *
	 * @see "BasePhyTests::planTests()" for details
	 */
	void testRun1(int stage, const cMessage* msg = 0);
	/**
	 * @brief Test handling for run 2:
	 * - check sending on already sending
	 */
	void testRun2(int stage, const cMessage* msg = 0);
	/**
	 * @brief Test handling for run 3:
	 * - check valid sending of a packet to 3 recipients
	 */
	void testRun3(int stage, const cMessage* msg = 0);

	/**
	 * @brief Test handling for run 5:
	 *
	 */
	void testRun5(int stage, const cMessage* msg = 0);

	/**
	 * @brief Test handling for run 6:
	 *
	 * @see "BasePhyTests::planTests()" for details
	 */
	void testRun6(int stage, const cMessage* msg = 0);

	/**
	 * @brief Test handling for run 7:
	 *
	 * @see "BasePhyTests::planTests()" for details
	 */
	void testRun7(int stage, const cMessage* msg = 0);

	//---run 3 tests------------------------------
	void testChannelInfo(int stage);
	void testSending1(int stage, const cMessage* lastMsg = 0);

	//---run 5 tests------------------------------
	void testChannelSenseWithBD();
	void testGetChannelStateWithBD();

	//---utilities--------------------------------
	/**
	 * @brief Continue with the next stage after the passed amount of seconds.
	 *
	 * @param time The amount of seconds to wait
	 */
	void continueIn(simtime_t_cref time);

	void waitForTX();
	void sendDown(MacPkt* pkt);
	void testForChannelSenseRequest(std::string test,
								    ChannelSenseRequest* req);
	MacPkt* createMacPkt(simtime_t_cref length);
};

#endif /*TESTMACLAYER_H_*/
