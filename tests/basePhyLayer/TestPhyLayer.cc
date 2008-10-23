#include "TestPhyLayer.h"
#include "../testUtils/asserts.h"

Define_Module(TestPhyLayer);

void TestPhyLayer::initialize(int stage) {
	if(stage == 0)
	{
		myIndex = findHost()->getIndex();

		// if parameter 'testBaseDecider' set, read it; otherwise set to false
		if ( hasPar("testBaseDecider") )
			testBaseDecider = par("testBaseDecider");
		else
			testBaseDecider = false;

	}

	//call BasePhy's initialize
	BasePhyLayer::initialize(stage);

	//run basic tests
	if(stage == 0) {
		init("phy" + toString(myIndex));

		// if we're testing the BaseDecider in this run,
		// initialize necessary members
		// NOTE: take care to call delete in stage 1 after tests when needed
		if (testBaseDecider)
		{
			// set controlling things (initial states)
			stateTestBDInitialization = BEFORE_TESTS;
			testRSSIMap = 0;
			testTime = 0;
			noAttenuation = 1.0;

			// some fix time-points for the signals
			t1 = 1;
			t3 = 3;
			t5 = 5;
			t7 = 7;
			t9 = 9;

			// time-points before and after
			before = 0;
			after = 10;

			// time-points in between
			t2 = 2;
			t4 = 4;
			t6 = 6;
			t8 = 8;

			// some TX-power values (should be 2^i, to detect errors in addition)
			TXpower1 = 1.0;
			TXpower2 = 2.0;
			TXpower3 = 4.0;

			// some bitrates
			bitrate9600 = 9600.0;


			// create test AirFrames
			// TODO: make sure that the AirFrames remain unchanged during all tests
			TestAF1 = createTestAirFrame(1);
			TestAF2 = createTestAirFrame(2);
			TestAF3 = createTestAirFrame(3);


			// expected results of tests (hard coded)
			res_t1_noisy = TXpower1;
			res_t2_noisy = TXpower1;
			res_t3_noisy = TXpower1 + TXpower2 + TXpower3;
			res_t4_noisy = TXpower1 + TXpower2 + TXpower3;
			res_t5_noisy = TXpower1 + TXpower2 + TXpower3;
			res_t9_noisy = TXpower2;

		}

	} else if(stage == 1) {
		testInitialisation();

		// if we're testing the BaseDecider in this run,
		// run specific BaseDecider tests (isolated) prior to simulation run
		if (testBaseDecider)
		{
			// start the test of BaseDecider (before simulation run)
			testBaseDeciderInitialization();

			// clean up
			delete TestAF1;
			TestAF1 = 0;
			delete TestAF2;
			TestAF2 = 0;
			delete TestAF3;
			TestAF3 = 0;
		}
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
	assertFalse("Check parameter \"usePropagationDelay\".", usePropagationDelay);

	assertEqual("Check parameter \"sensitivity\".", 2.0, sensitivity);
	assertEqual("Check parameter \"maxTXPower\".", 10.0, maxTXPower);
	assertEqual("Check parameter \"thermalNoise\".", 1.0, thermalNoise);

	assertTrue("Check upperGateIn ID.", upperGateIn != -1);
	assertTrue("Check upperGateOut ID.", upperGateOut != -1);
	assertTrue("Check upperControlIn ID.", upperControlIn != -1);
	assertTrue("Check upperControlOut ID.", upperControlOut != -1);

	//test radio state switching times
	radio->switchTo(Radio::SLEEP, simTime());
	radio->endSwitch(simTime());
	simtime_t swTime = radio->switchTo(Radio::RX, simTime());
	assertEqual("Switchtime SLEEP to RX.", 3.0, SIMTIME_DBL(swTime));
	radio->endSwitch(simTime());

	swTime = radio->switchTo(Radio::TX, simTime());
	assertEqual("Switchtime RX to TX.", 1.0, SIMTIME_DBL(swTime));
	radio->endSwitch(simTime());

	swTime = radio->switchTo(Radio::SLEEP, simTime());
	assertEqual("Switchtime TX to SLEEP.", 2.5, SIMTIME_DBL(swTime));
	radio->endSwitch(simTime());

	swTime = radio->switchTo(Radio::TX, simTime());
	assertEqual("Switchtime SLEEP to TX.", 3.5, SIMTIME_DBL(swTime));
	radio->endSwitch(simTime());

	swTime = radio->switchTo(Radio::RX, simTime());
	assertEqual("Switchtime TX to RX.", 2.0, SIMTIME_DBL(swTime));
	radio->endSwitch(simTime());

	swTime = radio->switchTo(Radio::SLEEP, simTime());
	assertEqual("Switchtime RX to SLEEP.", 1.5, SIMTIME_DBL(swTime));
	radio->endSwitch(simTime());

	// different if we use a TestBaseDecider, original code is in else-branch
	if (testBaseDecider)
	{
		TestBaseDecider* dec = dynamic_cast<TestBaseDecider*>(decider);
		assertTrue("Decider is of type TestBaseDecider.", dec != 0);
	}
	else
	{
		TestDecider* dec = dynamic_cast<TestDecider*>(decider);
		assertTrue("Decider is of type TestDecider.", dec != 0);
	}

	assertEqual("Check size of AnalogueModel list.", (size_t)3, analogueModels.size());
	double att1 = -1.0;
	double att2 = -1.0;

	// handling first Analogue Model (RSAM)
	AnalogueModelList::const_iterator it = analogueModels.begin();
	RadioStateAnalogueModel* model = dynamic_cast<RadioStateAnalogueModel*>(*it);
	assertTrue("Analogue model is of type RadioStateAnalogueModel.", model != 0);
	model = 0;
	it++;

	// handling all other analogue models
	for(; it != analogueModels.end(); it++) {

		TestAnalogueModel* model = dynamic_cast<TestAnalogueModel*>(*it);
		assertTrue("Analogue model is of type TestAnalogueModel.", model != 0);
		if(att1 < 0.0)
			att1 = model->att;
		else
			att2 = model->att;
	}

	assertTrue("Check attenuation value of AnalogueModels.",
				(FWMath::close(att1, 1.1) && FWMath::close(att2, 2.1))
				|| (FWMath::close(att1, 2.1) && FWMath::close(att2, 1.1)));

	//check initialisation of timers
	assertNotEqual("Check initialisation of TX-OVER timer", (void*)0, txOverTimer);
	assertEqual("Check kind of TX_OVER timer", TX_OVER, txOverTimer->getKind());

	assertNotEqual("Check initialisation of radioSwitchOver timer", (void*)0, radioSwitchingOverTimer);
	assertEqual("Check kind of radioSwitchOver timer", RADIO_SWITCHING_OVER, radioSwitchingOverTimer->getKind());

}

/*
 * Here we test the BaseDecider prior to simulation run.
 * We pass special values to TestBaseDecider to check its behavior.
 *
 * It is important to set 'stateTestBDInitialization' properly, such that
 * BaseDecider's calls on the DeciderToPhyInterface can be handled right.
 *
 *
 */
// TODO : finish
void TestPhyLayer::testBaseDeciderInitialization()
{
	assert(stateTestBDInitialization == BEFORE_TESTS);

	/*
	 * Test getChannelState() of BaseDecider on an empty channel
	 */
	stateTestBDInitialization = TEST_GET_CHANNELSTATE_EMPTYCHANNEL;
	doBaseDeciderTests();


	/*
	 * Test getChannelState() of BaseDecider with noise on channel
	 */
	stateTestBDInitialization = TEST_GET_CHANNELSTATE_NOISYCHANNEL;
	doBaseDeciderTests();


	// TODO: go on here...
}

AnalogueModel* TestPhyLayer::getAnalogueModelFromName(std::string name, ParameterMap& params) {

	AnalogueModel* model = BasePhyLayer::getAnalogueModelFromName(name, params);

	if (model != 0)
	{
		assertEqual("Check AnalogueModel name.", std::string("RadioStateAnalogueModel"), name);
		assertEqual("Check for correct RSAM-pointer.", radio->getAnalogueModel(), model);
		assertEqual("Check AnalogueModel parameter count.", (unsigned int)0, params.size());

		return model;
	}

	assertEqual("Check AnalogueModel name.", std::string("TestAnalogueModel"), name);

	assertEqual("Check AnalogueModel parameter count.", (unsigned int)1, params.size());

	assertEqual("Check for parameter \"attenuation\".", (unsigned int)1, params.count("attenuation"));
	cMsgPar par = params["attenuation"];
	assertEqual("Check type of parameter \"attenuation\".", 'D', par.getType());

	return new TestAnalogueModel(par.doubleValue());
}

Decider* TestPhyLayer::getDeciderFromName(std::string name, ParameterMap& params) {

	// testing a BaseDecider, so return an instance of TestBaseDecider
	if (testBaseDecider)
	{
		// check name
		assertEqual("Check Decider name.", std::string("TestBaseDecider"), name);

		// check number of parameters
		assertEqual("Check Decider parameter count.", (unsigned int)1, params.size());

		// check parameter 'snrThreshold'
		assertEqual("Check for parameter \"snrThreshold\".", (unsigned int)1, params.count("snrThreshold"));
		cMsgPar par = params["snrThreshold"];
		assertEqual("Check type of parameter \"snrThreshold\".", 'D', par.getType());
		assertEqual("Check value of parameter \"snrThreshold\".", -80.0, par.doubleValue());

		double snrThreshold = par.doubleValue();
		return new TestBaseDecider(this, snrThreshold, sensitivity, myIndex);
	}


	assertEqual("Check Decider name.", std::string("TestDecider"), name);

	assertEqual("Check Decider parameter count.", (unsigned int)8, params.size());

	assertEqual("Check for parameter \"aString\".", (unsigned int)1, params.count("aString"));
	cMsgPar par = params["aString"];
	assertEqual("Check type of parameter \"aString\".", 'S', par.getType());
	assertEqual("Check value of parameter \"aString\".", std::string("teststring"), std::string(par.stringValue()));

	assertEqual("Check for parameter \"anotherString\".", (unsigned int)1, params.count("anotherString"));
	par = params["anotherString"];
	assertEqual("Check type of parameter \"anotherString\".", 'S', par.getType());
	assertEqual("Check value of parameter \"anotherString\".", std::string("testString2"), std::string(par.stringValue()));

	assertEqual("Check for parameter \"aBool\".", (unsigned int)1, params.count("aBool"));
	par = params["aBool"];
	assertEqual("Check type of parameter \"aBool\".", 'B', par.getType());
	assertEqual("Check value of parameter \"aBool\".", true, par.boolValue());

	assertEqual("Check for parameter \"anotherBool\".", (unsigned int)1, params.count("anotherBool"));
	par = params["anotherBool"];
	assertEqual("Check type of parameter \"anotherBool\".", 'B', par.getType());
	assertEqual("Check value of parameter \"anotherBool\".", false, par.boolValue());

	assertEqual("Check for parameter \"aDouble\".", (unsigned int)1, params.count("aDouble"));
	par = params["aDouble"];
	assertEqual("Check type of parameter \"aDouble\".", 'D', par.getType());
	assertEqual("Check value of parameter \"aDouble\".", 0.25, par.doubleValue());

	assertEqual("Check for parameter \"anotherDouble\".", (unsigned int)1, params.count("anotherDouble"));
	par = params["anotherDouble"];
	assertEqual("Check type of parameter \"anotherDouble\".", 'D', par.getType());
	assertEqual("Check value of parameter \"anotherDouble\".", -13.52, par.doubleValue());

	assertEqual("Check for parameter \"aLong\".", (unsigned int)1, params.count("aLong"));
	par = params["aLong"];
	assertEqual("Check type of parameter \"aLong\".", 'L', par.getType());
	assertEqual("Check value of parameter \"aLong\".", 234567, par.longValue());

	assertEqual("Check for parameter \"anotherLong\".", (unsigned int)1, params.count("anotherLong"));
	par = params["anotherLong"];
	assertEqual("Check type of parameter \"anotherLong\".", 'L', par.getType());
	assertEqual("Check value of parameter \"anotherLong\".", -34567, par.longValue());

	int runNumber = simulation.getSystemModule()->par("run");

	return new TestDecider(this, myIndex, runNumber, RECEIVING);
}

/**
 * @brief Fills the AirFrameVector with intersecting AirFrames,
 * i.e. puts them on the (virtual) channel. (depends on testTime)
 *
 * AirFrames on the (virtual) channel in TestBDInitialization
 *
 * Frame1       |-----------------|				(start: t1, length: t7-t1)
 * Frame2             |-----------------|		(start: t3, length: t9-t3)
 * Frame3             |-----|					(start: t3, length: t5-t3)
 *
 *           |  |  |  |  |  |  |  |  |  |  |
 *           t0 t1 t2 t3 t4 t5 t6 t7 t8 t9 t10
 * To Test:  __ __ __ __ __ __          __
 *
 *           where: t0=before, t10=after
 */
void TestPhyLayer::fillAirFramesOnChannel()
{
	assert(testBaseDecider);


	switch (stateTestBDInitialization) {
		case TEST_GET_CHANNELSTATE_NOISYCHANNEL:

			// remove all pointers to AirFrames
			airFramesOnChannel.clear();

			// fill with pointers to test AirFrames that interfere with the testTime
			if (testTime == before || testTime == after)
			{
				// do nothing
			}
			else if (testTime == t1 || testTime == t2)
			{
				airFramesOnChannel.push_back(TestAF1);
			}
			else if (testTime == t3 || testTime == t4 || testTime == t5)
			{
				airFramesOnChannel.push_back(TestAF1);
				airFramesOnChannel.push_back(TestAF2);
				airFramesOnChannel.push_back(TestAF3);
			}
			else if (testTime == t6 || testTime == t7)
			{
				airFramesOnChannel.push_back(TestAF1);
				airFramesOnChannel.push_back(TestAF2);
			}
			else if (testTime == t8 || testTime == t9)
			{
				airFramesOnChannel.push_back(TestAF2);
			}
			else
			{
				// do nothing
			}

			break;
		default:
			break;
	}

	ev << TestModule::log("Filled airFramesOnChannel. (the virtual channel)") << endl;
}

/**
 * @brief Creates a special test AirFrame by index
 *
 * Below follow the parameters of the created signals, sorted by index.
 *
 * i=1: start: t1, length: t7-t1, TXpower: 1.0, bitrate: 9600.0, constant
 * i=2: start: t3, length: t9-t3, TXpower: 2.0, bitrate: 9600.0, constant
 * i=3: start: t3, length: t5-t3, TXpower: 3.0, bitrate: 9600.0, constant
 *
 * where TXpower and bitrate are also specified by a variable.
 *
 *
 * NOTE: No message is encapsulated in these test-AirFrames!
 */
AirFrame* TestPhyLayer::createTestAirFrame(int i)
{
	assert(testBaseDecider);

	// parameters needed
	simtime_t signalStart = -1;
	simtime_t signalLength = -1;
	double transmissionPower = 0;
	double bitrate = 0;

	// set parameters according to index
	switch (i) {
		case 1:
			signalStart = t1;
			signalLength = (t7-t1);
			transmissionPower = TXpower1;
			bitrate = bitrate9600;
			break;
		case 2:
			signalStart = t3;
			signalLength = (t9-t3);
			transmissionPower = TXpower2;
			bitrate = bitrate9600;
			break;
		case 3:
			signalStart = t3;
			signalLength = (t5-t3);
			transmissionPower = TXpower3;
			bitrate = bitrate9600;
			break;
		default:
			break;
	}

	/* Using convenience functions for creating the Signal

	// create and initialize Signal and AirFrame
	Signal* s = new Signal(signalStart, signalLength);

	// TODO : think of whether a Signal with constant power and bitrate
	// over the whole duration is sufficient for this test

	Mapping* powerMap = Mapping::createMapping();
	powerMap->setValue(Argument(signalStart), transmissionPower);
	s->setTransmissionPower(powerMap);

	Mapping* bitrateMap = Mapping::createMapping();
	bitrateMap->setValue(Argument(signalStart), bitrate);
	s->setBitrate(bitrateMap);

	*/

	// --- Mac-Layer's tasks

	// create Signal containing TXpower- and bitrate-mapping
	Signal* s = createSignal(signalStart, signalLength, transmissionPower, bitrate);

	// just a bypass attenuation, that has no effect on the TXpower
	Mapping* bypassMap = createConstantMapping(signalStart, signalStart + signalLength, noAttenuation);
	s->addAttenuation(bypassMap);

	// --- Phy-Layer's tasks

	// put host move pattern to Signal
	s->setMove(move);

	// create the new AirFrame
	AirFrame* frame = new AirFrame(0, AIR_FRAME);

	// set the members
	frame->setDuration(s->getSignalLength());
	// copy the signal into the AirFrame
	frame->setSignal(*s);

	// pointer and Signal not needed anymore
	delete s;
	s = 0;

	// TODO TEST: check if id is really unique
	frame->setId(world->getUniqueAirFrameId());

	ev << TestModule::log("Creating TestAirFrame ") << i << " done." << endl;

	return frame;
}

// pass AirFrame-pointers currently on the (virtual) channel to BaseDecider
void TestPhyLayer::passAirFramesOnChannel(AirFrameVector& out)
{
	assert(testBaseDecider);

	AirFrameVector::iterator it;

	// iterate over the member that holds the pointers to all AirFrames on
	// the virtual channel and put them to the output reference
	for (it = airFramesOnChannel.begin(); it != airFramesOnChannel.end(); it++)
	{
		out.push_back(*it);
	}

	ev << TestModule::log("All TestAirFrames-pointers have been copied.") << endl;
}

Signal* TestPhyLayer::createSignal(simtime_t start, simtime_t length, double power, double bitrate)
{
	simtime_t end = start + length;
	//create signal with start at current simtime and passed length
	Signal* s = new Signal(start, length);

	//create and set tx power mapping
	Mapping* txPowerMapping = createConstantMapping(start, end, power);
	s->setTransmissionPower(txPowerMapping);

	//create and set bitrate mapping
	Mapping* bitrateMapping = createConstantMapping(start, end, bitrate);
	s->setBitrate(bitrateMapping);

	return s;
}

Mapping* TestPhyLayer::createConstantMapping(simtime_t start, simtime_t end, double value)
{
	//create mapping over time
	Mapping* m = MappingUtils::createMapping(0.0, DimensionSet(Dimension::time), Mapping::LINEAR);

	//set position Argument
	Argument startPos(start);

	//set mapping at position
	m->setValue(startPos, value);

	//set position Argument
	Argument endPos(end);

	//set mapping at position
	m->setValue(endPos, value);

	return m;
}

// --- TESTING IMPLEMENTATION OF DeciderToPhyInterface !!! ---
// --- PLEASE REFER TO HEADER-FILE !!! ---

/**
 * SPECIAL TESTING IMPLEMENTATION: PLEASE REFER TO HEADER-FILE!
 */
void TestPhyLayer::getChannelInfo(simtime_t from, simtime_t to, AirFrameVector& out)
{
	Enter_Method_Silent();


	// if simulation is running or we are not testing the BaseDecider
	// behave like BasePhyLayer, otherwise do testing stuff (pass special things)
	if ( stateTestBDInitialization == SIMULATION_RUN
		 || !testBaseDecider)
	{
		BasePhyLayer::getChannelInfo(from, to, out);
		return;
	}

	switch (stateTestBDInitialization) {

		// there is no AirFrame on the Channel at the requested timepoint,
		// and we are not currently receiving an AirFrame
		case TEST_GET_CHANNELSTATE_EMPTYCHANNEL:
			// test whether values 'from' and 'to' are the same
			assertEqual( "Decider demands ChannelInfo for one single timepoint", from, to );
			assertEqual( "Decider demands ChannelInfo for current timepoint", from, getSimTime() );

			//  AirFrameVector referenced by 'out' remains as is
			break;

		// there are AirFrames on the Channel at the requested timepoint
		// but we are not currently receiving an AirFrame, so everything is noise
		case TEST_GET_CHANNELSTATE_NOISYCHANNEL:

			// test whether values 'from' and 'to' are the same
			assertEqual( "Decider demands ChannelInfo for one single timepoint", from, to );
			assertEqual( "Decider demands ChannelInfo for current test-timepoint", from, getSimTime() );

			// pass all AirFrames currently on the (virtual) channel
			passAirFramesOnChannel(out);



			break;
		// TODO go on here...

		default:
			break;
	}

	ev << TestModule::log("All channel-info has been copied to AirFrameVector-reference.") << endl;

	return;
}

/**
 * SPECIAL TESTING IMPLEMENTATION: PLEASE REFER TO HEADER-FILE!
 */
void TestPhyLayer::sendControlMsg(cMessage* msg)
{
	BasePhyLayer::sendControlMsg(msg);
	return;
}

/**
 * SPECIAL TESTING IMPLEMENTATION: PLEASE REFER TO HEADER-FILE!
 */
void TestPhyLayer::sendUp(AirFrame* packet, DeciderResult result)
{
	BasePhyLayer::sendUp(packet, result);
	return;
}

/**
 * SPECIAL TESTING IMPLEMENTATION: PLEASE REFER TO HEADER-FILE!
 */
simtime_t TestPhyLayer::getSimTime()
{
	// if we are not testing the BaseDecider
	// behave like BasePhyLayer, otherwise do testing stuff (pass special things)
	if (!testBaseDecider)
	{
		return BasePhyLayer::getSimTime();
	}

	switch (stateTestBDInitialization) {
		case SIMULATION_RUN:
		case TEST_GET_CHANNELSTATE_EMPTYCHANNEL:
			return BasePhyLayer::getSimTime();
			break;
		case TEST_GET_CHANNELSTATE_NOISYCHANNEL:
			return testTime;
			break;
		default:
			break;
	}

	return BasePhyLayer::getSimTime();
}

//
// --- implementation of BaseDecider-tests ---
//
void TestPhyLayer::doBaseDeciderTests()
{
	assert(testBaseDecider);

	ChannelState cs;


	switch (stateTestBDInitialization) {
		case TEST_GET_CHANNELSTATE_EMPTYCHANNEL:
		{
			// ask BaseDecider for the ChannelState, it should call getChannelInfo()
			// on the DeciderToPhyInterface
			testRSSIMap = MappingUtils::createMapping(0.0, DimensionSet(Dimension::time),
															Mapping::LINEAR);
			cs = decider->getChannelState();
			assertTrue("The PhyLayer is not currently receiving an AirFrame", cs.isIdle());
			assertEqual("RSSI-value is the value for 'now' in an empty Set of AirFrames (empty Mapping)", cs.getRSSI(),
					testRSSIMap->getValue(Argument(getSimTime())));

			delete testRSSIMap;
			testRSSIMap = 0;
		}
			break;

		case TEST_GET_CHANNELSTATE_NOISYCHANNEL:
		{
			ev << TestModule::log("------------------------------------------------") << endl;


			// 1. testTime = before = t0
			testTime = before;

			// this method-call depends on the variable testTime,
			// so it needs to be set properly before
			fillAirFramesOnChannel();

			testRSSIMap = MappingUtils::createMapping(0.0, DimensionSet(Dimension::time),
														Mapping::LINEAR);
			// call getChannelState() of the BaseDecider
			cs = decider->getChannelState();
			assertTrue("The PhyLayer is not currently receiving an AirFrame", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime' in an empty Set of AirFrames (empty Mapping)", cs.getRSSI(),
						testRSSIMap->getValue(Argument(getSimTime())));

			delete testRSSIMap;
			testRSSIMap = 0;

			ev << TestModule::log("-full output-----------------------------------------------") << endl;

			// 2. testTime = t1
			testTime = t1;
			ev << TestModule::log("A new testTime has been set: ")<< testTime << endl;

			ev << TestModule::log("Putting AirFrames on the virtual channel.") << endl;
			fillAirFramesOnChannel();

			ev << TestModule::log("Calling getChannelState() of BaseDecider.") << endl;
			cs = decider->getChannelState();

			ev << TestModule::log("Checking results against expected results.") << endl;
			assertTrue("The PhyLayer is not currently receiving an AirFrame", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime'", cs.getRSSI(), res_t1_noisy);

			ev << TestModule::log("-short output-----------------------------------------------") << endl;

			// 3. testTime = t2
			testTime = t2;
			ev << TestModule::log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();
			cs = decider->getChannelState();

			assertTrue("The PhyLayer is not currently receiving an AirFrame", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime'", cs.getRSSI(), res_t2_noisy);

			ev << TestModule::log("-short output-----------------------------------------------") << endl;

			// 4. testTime = t3
			testTime = t3;
			ev << TestModule::log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();
			cs = decider->getChannelState();

			assertTrue("The PhyLayer is not currently receiving an AirFrame", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime'", cs.getRSSI(), res_t3_noisy);

			ev << TestModule::log("-short output-----------------------------------------------") << endl;

			// 5. testTime = t4
			testTime = t4;
			ev << TestModule::log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();
			cs = decider->getChannelState();

			assertTrue("The PhyLayer is not currently receiving an AirFrame", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime'", cs.getRSSI(), res_t4_noisy);

			ev << TestModule::log("-short output-----------------------------------------------") << endl;

			// 6. testTime = t5
			testTime = t5;
			ev << TestModule::log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();
			cs = decider->getChannelState();

			assertTrue("The PhyLayer is not currently receiving an AirFrame", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime'", cs.getRSSI(), res_t5_noisy);

			ev << TestModule::log("-short output-----------------------------------------------") << endl;

			// 7. testTime = t9
			testTime = t9;
			ev << TestModule::log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();
			cs = decider->getChannelState();

			assertTrue("The PhyLayer is not currently receiving an AirFrame", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime'", cs.getRSSI(), res_t9_noisy);

		}
			break;
		default:
			break;
	}
}
