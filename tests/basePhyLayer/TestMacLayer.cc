#include "TestMacLayer.h"
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
 * - check switchRadio() method of MacToPhyInterface
 * - check sending on not TX mode
 * - check channel sense request
 */
void TestMacLayer::testRun1(int stage, const cMessage* msg){
	switch(stage) {
	case TEST_START:
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
		Radio::RadioState state = phy->getRadioState();
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
 */
void TestMacLayer::testRun3(int stage, const cMessage* msg){
	switch(myIndex) {
	case 0:
		testSending1(stage, msg);
		break;
	default:
		break;
	}
}

//---run 1 test-----------------------------
/**
 * Tests for switchRadio interface methods
 */
void TestMacLayer::testSwitchRadio(int stage) {
	Radio::RadioState expState;
	simtime_t expTime;
	Radio::RadioState nextState;
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

	Radio::RadioState state = phy->getRadioState();
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
	Radio::RadioState state = phy->getRadioState();
	assertNotEqual("Radio is not in TX.", Radio::TX, state);
	
	MacPkt* pkt = createMacPkt(1.0);
	sendDown(pkt);

	assertMessage("MacPkt at Phy layer.", TEST_MACPKT, simTime(), "phy0");
}

//---run 2 tests----------------------------


//---run 3 tests----------------------------

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
		
		assertMessage("First process of AirFrame at Decider", BasePhyLayer::AIR_FRAME, simTime(), "decider1");
		assertMessage("First process of AirFrame at Decider", BasePhyLayer::AIR_FRAME, simTime(), "decider2");
		assertMessage("First process of AirFrame at Decider", BasePhyLayer::AIR_FRAME, simTime(), "decider3");
				
		assertMessage("Transmission over message at phy", BasePhyLayer::TX_OVER, simTime() + 1.0, "phy0");
		assertMessage("Transmission over message from phy", BasePhyLayer::TX_OVER, simTime() + 1.0);
		break;
	}
	default:
		fail("Unknown stage");
		break;
	}
}

//---other----------------------------------
/**
 * tests for ChannelState interface methods
 */
void TestMacLayer::testChannelState() {
	//TODO: implement
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
