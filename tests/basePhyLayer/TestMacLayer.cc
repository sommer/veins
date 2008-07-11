#include "TestMacLayer.h"
#include <DeciderToPhyInterface.h>
Define_Module(TestMacLayer);

//---omnetpp part----------------------

//---intialisation---------------------
void TestMacLayer::initialize(int stage) {
	if(stage == 0) {
		myIndex = findHost()->index();
		
		dataOut = findGate("lowerGateOut");
		dataIn = findGate("lowerGateIn");
		
		controlOut = findGate("lowerControlOut");
		controlIn = findGate("lowerControlIn");
		
		init("mac" + toString(myIndex));
		
		testPhy = FindModule<TestPhyLayer*>::findSubModule(this->parentModule());
		phy = testPhy;
		
	} else if(stage == 1) {
		runTests(TEST_START);
	}
}

void TestMacLayer::onAssertedMessage(int state, const cMessage* msg) {
	runTests(state, msg);
}

//---test handling and redirection------------

/**
 * Redirects test handling to the "testRunx()"-methods
 * dependend on the current run.
 */
void TestMacLayer::runTests(int state, const cMessage* msg) {
	switch(simulation.runNumber()) {
	case 1:
		testRun1(state, msg);
		break;
	case 2:
		testRun2(state, msg);
		break;
	case 3:
		testRun3(state, msg);
		break;
	default:
		break;
	}
}

/**
 * Testhandling for run 1:
 * - check getChannelState()
 * - check switchRadio() method of MacToPhyInterface
 * - check sending on not TX mode
 * - check channel sense request
 */
void TestMacLayer::testRun1(int stage, const cMessage* msg){
	switch(stage) {
	case TEST_START:
		testGetChannelState();
	case RUN1_TEST_ON_RX:
		testSwitchRadio(stage);
		testChannelSense();
		break;
	case RUN1_TEST_ON_SLEEP:
		testSendingOnNotTX();
		break;
	default:
		fail("Invalid stage for run 1:" + toString(stage));
		break;
	}
}

/**
 * Testhandling for run 2:
 * - check sending on already sending
 */
void TestMacLayer::testRun2(int stage, const cMessage* msg){
	switch(stage) {
	case TEST_START:
		waitForTX(RUN2_SWITCH_TO_TX);
		break;
	case RUN2_SWITCH_TO_TX:{
		int state = phy->getRadioState();
		assertEqual("Radio is in TX.", Radio::TX, state);
		
		MacPkt* pkt = createMacPkt(1.0);
		sendDown(pkt);

		assertMessage("MacPkt at Phy layer.", TEST_MACPKT, simTime(), "phy0");
		//we don't assert any txOver message because we asume that the run
		//has been canceled before
		continueIn(0.5, RUN2_DURING_SENDING);
		break;
	}
	case RUN2_DURING_SENDING: {
		MacPkt* pkt = createMacPkt(1.0);
		sendDown(pkt);

		assertMessage("MacPkt at Phy layer.", TEST_MACPKT, simTime(), "phy0");
		//the run should be canceled after the asserted message, this is checked
		//indirectly by not asserting the txOverMessage
		break;
	}
	default:
		fail("Invalid stage for run 2:" + toString(stage));
		break;
	}
}

/**
 * Testhandling for run 3:
 * - check valid sending of a packet to 3 recipients
 * - check getChannelInfo
 */
void TestMacLayer::testRun3(int stage, const cMessage* msg){
	switch(myIndex) {
	case 0:
		testSending1(stage, msg);
		break;
	case 1:
	case 2:
	case 3:
		testChannelInfo(stage);
		break;
	default:
		fail("No handling for test run 3 for this host index: " + toString(myIndex));
		break;
	}
}

//---run 1 test-----------------------------

/**
 * Test the method "getChannelState()".
 */
void TestMacLayer::testGetChannelState() {
	ChannelState state = phy->getChannelState();
	assertTrue("First channelstates idle state should be true", state.isIdle());
	assertClose("First channelstates rssi.", 1.0, state.getRSSI());
	
	state = phy->getChannelState();
	assertFalse("Second channelstate should be false", state.isIdle());
	assertClose("Second channelstates rssi.", 2.0, state.getRSSI());
	
	state = phy->getChannelState();
	assertTrue("Third channelstate should be true", state.isIdle());
	assertClose("Third channelstates rssi.", 3.0, state.getRSSI());
	
	state = phy->getChannelState();
	assertFalse("Fourth channelstate should be false", state.isIdle());
	assertClose("Fourth channelstates rssi.", 4.0, state.getRSSI());
}
/**
 * Tests for switchRadio interface methods
 */
void TestMacLayer::testSwitchRadio(int stage) {
	int expState;
	simtime_t expTime;
	int nextState;
	int nextStage;
	
	switch(stage) {
	case TEST_START:
		expState = Radio::SLEEP;		
		nextState = Radio::RX;
		expTime = 3.0;
		nextStage = RUN1_TEST_ON_RX;
		break;
	case RUN1_TEST_ON_RX:
		expState = Radio::RX;		
		nextState = Radio::SLEEP;
		expTime = 1.5;
		nextStage = RUN1_TEST_ON_SLEEP;
		break;
		
	default:
		break;
	}

	int state = phy->getRadioState();
	assertEqual("Radio starts in SLEEP.", expState, state);
	simtime_t switchTime = phy->setRadioState(nextState);
	assertEqual("Correct switch time to RX.", expTime, switchTime);
	
	
	assertMessage(	"SWITCH_OVER message at phy.", 
					BasePhyLayer::RADIO_SWITCHING_OVER, 
					simTime() + switchTime, 
					"phy" + toString(myIndex));
	waitForMessage(	nextStage,
					"SWITCH_OVER message.", 
					BasePhyLayer::RADIO_SWITCHING_OVER, 
					simTime() + switchTime);
	
	switchTime = phy->setRadioState(Radio::RX);
	assertTrue("Invalid switchtime because already switching.", switchTime < 0.0);	
}

void TestMacLayer::testChannelSense() {
	ChannelSenseRequest* req = new ChannelSenseRequest();
	req->setKind(MacToPhyInterface::CHANNEL_SENSE_REQUEST);
	req->setSenseDuration(0.5f);
	send(req, controlOut);	
	assertMessage(	"ChannelSense at phy layer.", 
					MacToPhyInterface::CHANNEL_SENSE_REQUEST,
					simTime(), "phy" + toString(myIndex));
	assertMessage(	"ChannelSense at decider.", 
						MacToPhyInterface::CHANNEL_SENSE_REQUEST,
						simTime(), "decider" + toString(myIndex));
}

void TestMacLayer::testSendingOnNotTX() {
	int state = phy->getRadioState();
	assertNotEqual("Radio is not in TX.", Radio::TX, state);
	
	MacPkt* pkt = createMacPkt(1.0);
	sendDown(pkt);

	assertMessage("MacPkt at Phy layer.", TEST_MACPKT, simTime(), "phy0");
}

//---run 2 tests----------------------------


//---run 3 tests----------------------------

void TestMacLayer::testChannelInfo(int stage) {
	switch(stage) {
	case TEST_START: {
		DeciderToPhyInterface::AirFrameVector v;
		testPhy->getChannelInfo(0.0, simTime(), v);
		
		assertTrue("No AirFrames on channel.", v.empty());		
		break;
	}
	case RUN3_TEST_ON_DECIDER1:
	case RUN3_TEST_ON_DECIDER2:
	case RUN3_TEST_ON_DECIDER3:{
		if(myIndex ==( stage - RUN3_TEST_ON_DECIDER1 + 1)) {
			DeciderToPhyInterface::AirFrameVector v;
			testPhy->getChannelInfo(0.0, simTime(), v);
			
			assertFalse("AirFrames on channel.", v.empty());	
		}
		break;
	}
	default:
		fail("TestChannelInfo: Unknown stage.");
		break;
	}
	//displayPassed = false;
}

void TestMacLayer::testSending1(int stage, const cMessage* lastMsg) {
	switch(stage) {
	case TEST_START: {
		waitForTX(RUN3_TEST_ON_TX);
		
		break;
	}
	case RUN3_TEST_ON_TX:{
		MacPkt* pkt = createMacPkt(1.0);

		sendDown(pkt);

		assertMessage("MacPkt at Phy layer.", TEST_MACPKT, simTime(), "phy0");
		
		assertMessage("First receive of AirFrame", BasePhyLayer::AIR_FRAME, simTime(), "phy1");
		assertMessage("First receive of AirFrame", BasePhyLayer::AIR_FRAME, simTime(), "phy2");
		assertMessage("First receive of AirFrame", BasePhyLayer::AIR_FRAME, simTime(), "phy3");
		assertMessage("End receive of AirFrame", BasePhyLayer::AIR_FRAME, simTime() + 1.0, "phy1");
		assertMessage("End receive of AirFrame", BasePhyLayer::AIR_FRAME, simTime() + 1.0, "phy2");
		assertMessage("End receive of AirFrame", BasePhyLayer::AIR_FRAME, simTime() + 1.0, "phy3");
		
		waitForMessage(RUN3_TEST_ON_DECIDER1, "First process of AirFrame at Decider", BasePhyLayer::AIR_FRAME, simTime(), "decider1");
		waitForMessage(RUN3_TEST_ON_DECIDER2, "First process of AirFrame at Decider", BasePhyLayer::AIR_FRAME, simTime(), "decider2");
		waitForMessage(RUN3_TEST_ON_DECIDER3, "First process of AirFrame at Decider", BasePhyLayer::AIR_FRAME, simTime(), "decider3");
				
		assertMessage("Transmission over message at phy", BasePhyLayer::TX_OVER, simTime() + 1.0, "phy0");
		assertMessage("Transmission over message from phy", BasePhyLayer::TX_OVER, simTime() + 1.0);
		break;
	}
	case RUN3_TEST_ON_DECIDER1:
	case RUN3_TEST_ON_DECIDER2:
	case RUN3_TEST_ON_DECIDER3:{
		TestMacLayer* mac = dynamic_cast<TestMacLayer*>(manager->getModule("mac" + toString((stage - RUN3_TEST_ON_DECIDER1)+ 1)));
		mac->onAssertedMessage(stage, 0);
		break;
	}
	default:
		fail("Unknown stage");
		break;
	}
}

//---utilities------------------------------

void TestMacLayer::continueIn(simtime_t time, int nextStage){
	scheduleAt(simTime() + time, new cMessage(0, 232425));
	waitForMessage(	nextStage, 
					"Waiting for " + toString(time) + "s.", 
					232425,
					simTime() + time);
}

void TestMacLayer::waitForTX(int nextStage) {
	simtime_t switchTime = phy->setRadioState(Radio::TX);
	assertTrue("A valid switch time.", switchTime >= 0.0);
	
	
	assertMessage(	"SWITCH_OVER to TX message at phy.", 
					BasePhyLayer::RADIO_SWITCHING_OVER, 
					simTime() + switchTime, 
					"phy" + toString(myIndex));
	waitForMessage(	nextStage,
					"SWITCH_OVER to TX message.", 
					BasePhyLayer::RADIO_SWITCHING_OVER, 
					simTime() + switchTime);
}

void TestMacLayer::sendDown(MacPkt* pkt) {
	send(pkt, dataOut);
}

MacPkt* TestMacLayer::createMacPkt(simtime_t length) {
	Signal* s = new Signal(simTime(), length);
	MacToPhyControlInfo* ctrl = new MacToPhyControlInfo(s);
	MacPkt* res = new MacPkt();
	res->setControlInfo(ctrl);
	res->setKind(TEST_MACPKT);
	return res;
}
