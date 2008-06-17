#include "TestPhyLayer.h"
#include "../testUtils/asserts.h"

Define_Module(TestPhyLayer);

void TestPhyLayer::initialize(int stage) {
	displayPassed = false;
	if(stage == 0)
		myIndex = findHost()->index();
	
	//call BasePhy's initialize
	BasePhyLayer::initialize(stage);
	
	//run basic tests
	if(stage == 0) {
		init("phy" + toString(myIndex));
	} else if(stage == 1) {
		testInitialisation();
	}
}

void TestPhyLayer::handleMessage(cMessage* msg) {
	announceMessage(msg);
	
	BasePhyLayer::handleMessage(msg);	
}

TestPhyLayer::~TestPhyLayer() {
	finalize();
}

void TestPhyLayer::testInitialisation() {
	
	//run dependend tests
	switch(simulation.runNumber()) {
	default:
		assertFalse("Check parameter \"usePropagationDelay\".", usePropagationDelay);
		break;
	}
	
		
	assertEqual("Check parameter \"sensitivity\".", 2.0, sensitivity);
	assertEqual("Check parameter \"maxTXPower\".", 10.0, maxTXPower);
	assertEqual("Check parameter \"thermalNoise\".", 1.0, thermalNoise);
	
	assertTrue("Check upperGateIn ID.", upperGateIn != -1);
	assertTrue("Check upperGateOut ID.", upperGateOut != -1);
	assertTrue("Check upperControlIn ID.", upperControlIn != -1);
	assertTrue("Check upperControlOut ID.", upperControlOut != -1);
	
	//test radio state switching times
	radio.switchTo(Radio::SLEEP, simTime());
	radio.endSwitch(simTime());
	simtime_t swTime = radio.switchTo(Radio::RX, simTime());
	assertEqual("Switchtime SLEEP to RX.", 3.0, swTime);
	radio.endSwitch(simTime());
	
	swTime = radio.switchTo(Radio::TX, simTime());
	assertEqual("Switchtime RX to TX.", 1.0, swTime);
	radio.endSwitch(simTime());
	
	swTime = radio.switchTo(Radio::SLEEP, simTime());
	assertEqual("Switchtime TX to SLEEP.", 2.5, swTime);
	radio.endSwitch(simTime());
	
	swTime = radio.switchTo(Radio::TX, simTime());
	assertEqual("Switchtime SLEEP to TX.", 3.5, swTime);
	radio.endSwitch(simTime());
	
	swTime = radio.switchTo(Radio::RX, simTime());
	assertEqual("Switchtime TX to RX.", 2.0, swTime);
	radio.endSwitch(simTime());
	
	swTime = radio.switchTo(Radio::SLEEP, simTime());
	assertEqual("Switchtime RX to SLEEP.", 1.5, swTime);
	radio.endSwitch(simTime());
	
	TestDecider* dec = dynamic_cast<TestDecider*>(decider);
	assertTrue("Decider is of type TestDecider.", dec != 0);
	
	assertEqual("Check size of AnalogueModel list.", (size_t)2, analogueModels.size());
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
	
	//check initialisation of timers
	assertNotEqual("Check initialisation of TX-OVER timer", (void*)0, txOverTimer);
	assertEqual("Check kind of TX_OVER timer", TX_OVER, txOverTimer->kind());
	
	assertNotEqual("Check initialisation of radioSwitchOver timer", (void*)0, radioSwitchingOverTimer);
	assertEqual("Check kind of radioSwitchOver timer", RADIO_SWITCHING_OVER, radioSwitchingOverTimer->kind());
	
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
	
	return new TestDecider(this, myIndex, simulation.runNumber(), RECEIVING);
}
