#include "TestPhyLayer.h"

Define_Module(TestPhyLayer);

template<class T> std::string stringify(T x)
{
	std::ostringstream o;
	o << x;
	return o.str();
}


void TestPhyLayer::initialize(int stage) {
	
	//call BasePhy's initialize
	BasePhyLayer::initialize(stage);
	
	//run basic tests
	if(stage == 1) {
		testInitialisation();
		testMacToPhyInterface();
		testDeciderToPhyInterface();
		testHandleMessage();
	}
}

void TestPhyLayer::handleMessage(cMessage* msg) {
	
	BasePhyLayer::handleMessage(msg);	
	
	simtime_t time = simTime();
	MessageMap::iterator it = expectedMsgs.begin();
	
	while(it != expectedMsgs.lower_bound(time)) {
		
		fail("Expected message - \"" + it->second.second, it->first, time);
		expectedMsgs.erase(it++);
	}
	bool expected;
	while(it != expectedMsgs.upper_bound(time)) {
		if(it->second.first == msg->kind()) {
			pass("Expected message - \"" + it->second.second + "\" at " + stringify(time));
			expectedMsgs.erase(it++);
			expected = true;
			break;
		} else {
			it++;
		}
	}
	if(!expected)
		fail("Got unexpected message with kind " + stringify(msg->kind()) + " at " + stringify(time));
}

void TestPhyLayer::testMacToPhyInterface() {
	
	//get and set radiostate
	Radio::RadioState state = getRadioState();
	assertEqual("Radio starts in SLEEP.", Radio::SLEEP, state);
	simtime_t switchTime = setRadioState(Radio::RX);
	assertEqual("Correct switch time to RX.", 3.0, switchTime);
	assertMessage("Expect SWITCH_OVER to RX.", RADIO_SWITCHING_OVER, simTime() + switchTime);
	
	switchTime = setRadioState(Radio::RX);
	assertTrue("Invalid switchtime because already switching.", switchTime < 0.0);	
	
	//getChannelState
	//TODO: implement
	
}

void TestPhyLayer::testDeciderToPhyInterface() {
	//TODO: implement
	
}
	
void TestPhyLayer::testHandleMessage() {
	//TODO implement
}

void TestPhyLayer::testInitialisation() {
	
	assertTrue("Check parameter \"usePropagationDelay\".", usePropagationDelay);
		
	assertEqual("Check parameter \"sensitivity\".", 2.0, sensitivity);
	assertEqual("Check parameter \"maxTXPower\".", 10.0, maxTXPower);
	assertEqual("Check parameter \"thermalNoise\".", 1.0, thermalNoise);
	
	//test radio state switching times
	radio.switchTo(Radio::SLEEP);
	radio.endSwitch();
	simtime_t swTime = radio.switchTo(Radio::RX);
	assertEqual("Switchtime SLEEP to RX.", 3.0, swTime);
	radio.endSwitch();
	
	swTime = radio.switchTo(Radio::TX);
	assertEqual("Switchtime RX to TX.", 1.0, swTime);
	radio.endSwitch();
	
	swTime = radio.switchTo(Radio::SLEEP);
	assertEqual("Switchtime TX to SLEEP.", 2.5, swTime);
	radio.endSwitch();
	
	swTime = radio.switchTo(Radio::TX);
	assertEqual("Switchtime SLEEP to TX.", 3.5, swTime);
	radio.endSwitch();
	
	swTime = radio.switchTo(Radio::RX);
	assertEqual("Switchtime TX to RX.", 2.0, swTime);
	radio.endSwitch();
	
	swTime = radio.switchTo(Radio::SLEEP);
	assertEqual("Switchtime RX to SLEEP.", 1.5, swTime);
	radio.endSwitch();
	
	TestDecider* dec = dynamic_cast<TestDecider*>(decider);
	assertTrue("Decider is of type TestDecider.", dec != 0);
	
	assertEqual("Check size of AnalogueModel list.", 2, analogueModels.size());
	double att1 = -1.0;
	double att2 = -1.0;
	for(AnalogueModelList::const_iterator it = analogueModels.begin();
		it != analogueModels.end(); it++) {
		
		TestAnalogueModel* model = dynamic_cast<TestAnalogueModel*>(*it);
		assertTrue("Analogue model is of type TestAnalogueModel.", model != 0);
		if(att1 < 0.0)
			att1 = model->att;
		else
			att2 = model->att;
	}
	
	assertTrue("Check attenuation value of AnalogueModels.", 
				FWMath::close(att1, 1.1) && FWMath::close(att2, 2.1)
				|| FWMath::close(att1, 2.1) && FWMath::close(att2, 1.1));
}

AnalogueModel* TestPhyLayer::getAnalogueModelFromName(std::string name, ParameterMap& params) {
	assertEqual("Check AnalogueModel name.", std::string("TestAnalogueModel"), name);
	
	assertEqual("Check AnalogueModel parameter count.", 1, params.size());
	
	assertEqual("Check for parameter \"attenuation\".", 1, params.count("attenuation"));
	cPar par = params["attenuation"];
	assertEqual("Check type of parameter \"attenuation\".", 'D', par.type());
		
	return new TestAnalogueModel(par.doubleValue());
}
		
Decider* TestPhyLayer::getDeciderFromName(std::string name, ParameterMap& params) {
	assertEqual("Check Decider name.", std::string("TestDecider"), name);
		
	assertEqual("Check Decider parameter count.", 8, params.size());
	
	assertEqual("Check for parameter \"aString\".", 1, params.count("aString"));
	cPar par = params["aString"];
	assertEqual("Check type of parameter \"aString\".", 'S', par.type());
	assertEqual("Check value of parameter \"aString\".", std::string("teststring"), std::string(par.stringValue()));
	
	assertEqual("Check for parameter \"anotherString\".", 1, params.count("anotherString"));
	par = params["anotherString"];
	assertEqual("Check type of parameter \"anotherString\".", 'S', par.type());
	assertEqual("Check value of parameter \"anotherString\".", std::string("testString2"), std::string(par.stringValue()));
	
	assertEqual("Check for parameter \"aBool\".", 1, params.count("aBool"));
	par = params["aBool"];
	assertEqual("Check type of parameter \"aBool\".", 'B', par.type());
	assertEqual("Check value of parameter \"aBool\".", true, par.boolValue());
	
	assertEqual("Check for parameter \"anotherBool\".", 1, params.count("anotherBool"));
	par = params["anotherBool"];
	assertEqual("Check type of parameter \"anotherBool\".", 'B', par.type());
	assertEqual("Check value of parameter \"anotherBool\".", false, par.boolValue());
	
	assertEqual("Check for parameter \"aDouble\".", 1, params.count("aDouble"));
	par = params["aDouble"];
	assertEqual("Check type of parameter \"aDouble\".", 'D', par.type());
	assertEqual("Check value of parameter \"aDouble\".", 0.25, par.doubleValue());
	
	assertEqual("Check for parameter \"anotherDouble\".", 1, params.count("anotherDouble"));
	par = params["anotherDouble"];
	assertEqual("Check type of parameter \"anotherDouble\".", 'D', par.type());
	assertEqual("Check value of parameter \"anotherDouble\".", -13.52, par.doubleValue());
	
	assertEqual("Check for parameter \"aLong\".", 1, params.count("aLong"));
	par = params["aLong"];
	assertEqual("Check type of parameter \"aLong\".", 'L', par.type());
	assertEqual("Check value of parameter \"aLong\".", 234567, par.longValue());
	
	assertEqual("Check for parameter \"anotherLong\".", 1, params.count("anotherLong"));
	par = params["anotherLong"];
	assertEqual("Check type of parameter \"anotherLong\".", 'L', par.type());
	assertEqual("Check value of parameter \"anotherLong\".", -34567, par.longValue());
	
	return new TestDecider();
}
