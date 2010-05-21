#include "DeciderTest.h"
#include "../testUtils/asserts.h"

Define_Module(DeciderTest);

DeciderTest::DeciderTest()
{
	// set the testBaseDecider flag to true, because this class
	// only does testing
	testBaseDecider = true;

	// initializing members for testing
	move = Move();
	world = new TestWorld();


	// former initialization (stage == 0)

	myIndex = 0;

	//run basic tests

	// set controlling things (initial states)
	stateTestBDInitialization = BEFORE_TESTS;
	testRSSIMap = 0;
	testTime = 0;
	noAttenuation = 1.0;
	sendUpCalled = false;

	// some fix time-points for the signals
	t0 = 0;
	t1 = 1;
	t3 = 3;
	t5 = 5;
	t7 = 7;
	t9 = 9;

	// time-points before and after
	before = t0;
	after = 10;

	// time-points in between
	t2 = 2;
	t4 = 4;
	t6 = 6;
	t8 = 8;

	// some TX-power values (should be 2^i, to detect errors in addition)
	TXpower1 = 1.0;
	TXpower2 = 3.981071705534;
	TXpower3 = 3.981071705535; // == 6dBm (sensitivity)
	TXpower4 = 8.0;

	// TX-power for SNR-threshold-test AirFrame
	TXpower5P = 16.0;
	TXpower5H = 32.0;

	TXpower6 = 1.0;

	// some bitrates
	bitrate9600 = 9600.0;


	// create test AirFrames
	// TODO: make sure that the AirFrames remain unchanged during all tests
	TestAF1 = createTestAirFrame(1);
	TestAF2 = createTestAirFrame(2);
	TestAF3 = createTestAirFrame(3);
	TestAF4 = createTestAirFrame(4);

	// AirFrames for SNR-threshold testing
	TestAF5 = createTestAirFrame(5);
	TestAF6 = createTestAirFrame(6);


	processedAF = 0;
	testChannelSense = 0;


	// expected results of tests (hard coded)
	res_t1_noisy = TXpower1;
	res_t2_noisy = TXpower1;
	res_t3_noisy = TXpower1 + TXpower2 + TXpower3;
	res_t4_noisy = TXpower1 + TXpower2 + TXpower3;
	res_t5_noisy = TXpower1 + TXpower2 + TXpower3 + TXpower4;
	res_t9_noisy = TXpower2;

	// expected results of getChannelState()-tests, when receiving TestAirFrame 3
	// (testing the exclusion of a Frame when calculating the RSSI-value)
	res_t2_receiving		= TXpower1;
	res_t3_receiving_before	= TXpower1 + TXpower2 + TXpower3;
	res_t3_receiving_after	= TXpower1 + TXpower2 + TXpower3;
	res_t4_receiving		= TXpower1 + TXpower2 + TXpower3;
	res_t5_receiving_before	= TXpower1 + TXpower2 + TXpower3	+ TXpower4;
	res_t5_receiving_after	= TXpower1 + TXpower2 + TXpower3 	+ TXpower4;
	res_t6_receiving		= TXpower1 + TXpower2 				+ TXpower4;


}



DeciderTest::~DeciderTest() {

	// clean up
	delete TestAF1;
	TestAF1 = 0;
	delete TestAF2;
	TestAF2 = 0;
	delete TestAF3;
	TestAF3 = 0;
	delete TestAF4;
	TestAF4 = 0;
	delete TestAF5;
	TestAF5 = 0;
	delete TestAF6;
	TestAF6 = 0;

	delete world;
	world = 0;
}



/*
 * Here we test the SNRThresholdDecider prior to simulation run.
 * We pass special values to TestBaseDecider to check its behavior.
 *
 * It is important to set 'stateTestBDInitialization' properly, such that
 * SNRThresholdDecider's calls on the DeciderToPhyInterface can be handled right.
 *
 *
 */
// TODO : finish
void DeciderTest::testBaseDeciderInitialization()
{
	assert(stateTestBDInitialization == BEFORE_TESTS);

	/*
	 * Test getChannelState() of SNRThresholdDecider on an empty channel
	 */
	stateTestBDInitialization = TEST_GET_CHANNELSTATE_EMPTYCHANNEL;
	doBaseDeciderTests();


	/*
	 * Test getChannelState() of SNRThresholdDecider with noise on channel
	 */
	stateTestBDInitialization = TEST_GET_CHANNELSTATE_NOISYCHANNEL;
	doBaseDeciderTests();


	/*
	 * Test getChannelState() of SNRThresholdDecider while receiving an AirFrame
	 */
	stateTestBDInitialization = TEST_GET_CHANNELSTATE_RECEIVING;
	doBaseDeciderTests();

	// TODO: go on here...
	// TODO	- test whether SNRThresholdDecider calculates SNR for a given signal correctly
	//		- test whether SNRThresholdDecider handles SNR-threshold right (send up a packet or not)

	/*
	 * Test SNR-threshold
	 */
	stateTestBDInitialization = TEST_SNR_THRESHOLD_ACCEPT;
	doBaseDeciderTests();

	stateTestBDInitialization = TEST_SNR_THRESHOLD_DENY;
	doBaseDeciderTests();


	stateTestBDInitialization = TEST_SNR_THRESHOLD_PAYLOAD_DENY;
	doBaseDeciderTests();


	stateTestBDInitialization = TEST_SNR_THRESHOLD_MORE_NOISE_BEGINS_IN_BETWEEN_DENY;
	doBaseDeciderTests();


	stateTestBDInitialization = TEST_SNR_THRESHOLD_NOISE_ENDS_AT_BEGINNING_DENY;
	doBaseDeciderTests();


	stateTestBDInitialization = TEST_SNR_THRESHOLD_NOISE_BEGINS_AT_END_DENY;
	doBaseDeciderTests();

	stateTestBDInitialization = TEST_CHANNELSENSE;
	doBaseDeciderTests();

}



Decider* DeciderTest::getDeciderFromName(std::string name, ParameterMap& params) {

	// TODO: extend to use the passed parameters to initialize multiple Deciders (switch)

	// parameters for original TestBaseDecider (tests/basePhyLayer with testBaseDecider = true)
	double snrThreshold = 10.0;
	double sensitivity = FWMath::dBm2mW(6.0);
	bool coreDebug = false;

	return new TestBaseDecider(this, snrThreshold, sensitivity, myIndex, coreDebug);
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
 * Frame4                   |--|				(start: t5, length: t6-t5)
 * SNR-Frame    |xx|--------|					(start: t1, length: t5-t1, header (xx): t2-t1)
 * (Frame5)
 * Frame6    |--|								(start: t0, length: t1-t0)
 *           |  |  |  |  |  |  |  |  |  |  |
 *           t0 t1 t2 t3 t4 t5 t6 t7 t8 t9 t10
 * To Test:  __ __ __ __ __ __          __
 *
 *           where: t0=before, t10=after
 */
// TODO implement TestAirFrame6
void DeciderTest::fillAirFramesOnChannel()
{
	assert(testBaseDecider);

	// remove all pointers to AirFrames
	airFramesOnChannel.clear();
	if(TestAF5)
		delete TestAF5;

	TestAF5 = createTestAirFrame(5);
	switch (stateTestBDInitialization) {


		// TODO put the right AirFrames on the channel for the following cases
		case TEST_SNR_THRESHOLD_ACCEPT:
		case TEST_SNR_THRESHOLD_DENY:
		case TEST_SNR_THRESHOLD_PAYLOAD_DENY:
			airFramesOnChannel.push_back(TestAF1);
			airFramesOnChannel.push_back(TestAF5);
			break;

		case TEST_SNR_THRESHOLD_MORE_NOISE_BEGINS_IN_BETWEEN_DENY:
			airFramesOnChannel.push_back(TestAF1);
			airFramesOnChannel.push_back(TestAF2);
			airFramesOnChannel.push_back(TestAF5);
			break;

		case TEST_SNR_THRESHOLD_NOISE_ENDS_AT_BEGINNING_DENY:
			airFramesOnChannel.push_back(TestAF1);
			airFramesOnChannel.push_back(TestAF5);
			airFramesOnChannel.push_back(TestAF6);
			break;

		case TEST_SNR_THRESHOLD_NOISE_BEGINS_AT_END_DENY:
			airFramesOnChannel.push_back(TestAF1);
			airFramesOnChannel.push_back(TestAF4);
			airFramesOnChannel.push_back(TestAF5);
			break;
		// TODO end.


		case TEST_GET_CHANNELSTATE_NOISYCHANNEL:
		case TEST_GET_CHANNELSTATE_RECEIVING:
		case TEST_CHANNELSENSE:


			// fill with pointers to test AirFrames that interfere with the testTime
			if (testTime == before || testTime == after)
			{
				// do nothing
			}
			else if (testTime == t1 || testTime == t2)
			{
				airFramesOnChannel.push_back(TestAF1);
			}
			else if (testTime == t3 || testTime == t4)
			{
				airFramesOnChannel.push_back(TestAF1);
				airFramesOnChannel.push_back(TestAF2);
				airFramesOnChannel.push_back(TestAF3);

			}
			else if (testTime == t5)
			{
				airFramesOnChannel.push_back(TestAF1);
				airFramesOnChannel.push_back(TestAF2);
				airFramesOnChannel.push_back(TestAF3);
				airFramesOnChannel.push_back(TestAF4);

			}
			else if (testTime == t6)
			{
				airFramesOnChannel.push_back(TestAF1);
				airFramesOnChannel.push_back(TestAF2);
				airFramesOnChannel.push_back(TestAF4);
			}

			else if (testTime == t7)
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

	ev << log("Filled airFramesOnChannel. (the virtual channel)") << endl;
}

/**
 * @brief Creates a special test AirFrame by index
 *
 * Below follow the parameters of the created signals, sorted by index.
 *
 * i=1: start: t1, length: t7-t1, TXpower: 1.0, bitrate: 9600.0, constant
 * i=2: start: t3, length: t9-t3, TXpower: 2.0, bitrate: 9600.0, constant
 * i=3: start: t3, length: t5-t3, TXpower: 4.0, bitrate: 9600.0, constant
 * i=4: start: t5, length: t6-t5, TXpower: 8.0, bitrate: 9600.0, constant
 *
 * This is the AirFrame for SNR-threshold testing, its values may be changed during test
 * i=5: start: t1, length: t5-t1, TXpower: variable, bitrate: 9600.0, steps
 *
 * i=6:	start: t0, length : t1-t0, TXpower 1.0, bitrate: 9600.0, constant
 *
 * where TXpower and bitrate are also specified by a variable.
 *
 *
 * NOTE: No message is encapsulated in these test-AirFrames!
 */
AirFrame* DeciderTest::createTestAirFrame(int i)
{
	assert(testBaseDecider);

	// parameters needed
	simtime_t signalStart = -1;
	simtime_t signalLength = -1;
	// values for header/payload
	std::pair<double, double> transmissionPower(0, 0);
	std::pair<double, double> bitrate(0, 0);
	simtime_t payloadStart = -1;

	// set parameters according to index
	switch (i) {
		case 1:
			signalStart = t1;
			signalLength = (t7-t1);
			transmissionPower.first = TXpower1;
			bitrate.first = bitrate9600;
			break;
		case 2:
			signalStart = t3;
			signalLength = (t9-t3);
			transmissionPower.first = TXpower2;
			bitrate.first = bitrate9600;
			break;
		case 3:
			signalStart = t3;
			signalLength = (t5-t3);
			transmissionPower.first = TXpower3;
			bitrate.first = bitrate9600;
			break;
		case 4:
			signalStart = t5;
			signalLength = (t6-t5);
			transmissionPower.first = TXpower4;
			bitrate.first = bitrate9600;
			break;
		case 5:
			signalStart = t1;
			signalLength = (t5-t1);

			switch (stateTestBDInitialization) {
				case TEST_SNR_THRESHOLD_ACCEPT:
					transmissionPower.first = 10.00001;
					transmissionPower.second = 10.00001;
					break;
				case TEST_SNR_THRESHOLD_DENY:
					transmissionPower.first = 10.0;
					transmissionPower.second = 10.0;
					break;
				case TEST_SNR_THRESHOLD_PAYLOAD_DENY:
					transmissionPower.first = 20.0;
					transmissionPower.second = 10.0;
					break;
				case TEST_SNR_THRESHOLD_MORE_NOISE_BEGINS_IN_BETWEEN_DENY:
					transmissionPower.first = 30.0;
					transmissionPower.second = 30.0;
					break;
				case TEST_SNR_THRESHOLD_NOISE_ENDS_AT_BEGINNING_DENY:
					transmissionPower.first = 20.0;
					transmissionPower.second = 20.0;
					break;
				case TEST_SNR_THRESHOLD_NOISE_BEGINS_AT_END_DENY:
					transmissionPower.first = 90.0;
					transmissionPower.second = 90.0;
					break;
				default:
					transmissionPower.first = TXpower5H;
					transmissionPower.second = TXpower5P;
					break;
			}

			bitrate.first = bitrate9600;
			payloadStart = t2;
			break;
		case 6:
			signalStart = t0;
			signalLength = (t1-t0);
			transmissionPower.first = TXpower6;
			bitrate.first = bitrate9600;
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
	Signal* s = createSignal(signalStart, signalLength, transmissionPower, bitrate, i, payloadStart);

	// just a bypass attenuation, that has no effect on the TXpower
	Mapping* bypassMap = createConstantMapping(signalStart, signalStart + signalLength, noAttenuation);
	s->addAttenuation(bypassMap);

	// --- Phy-Layer's tasks

	// put host move pattern to Signal
	s->setMove(move);

	// create the new AirFrame
	AirFrame* frame = new AirFrame(0, MacToPhyInterface::AIR_FRAME);

	// set the members
	frame->setDuration(s->getSignalLength());
	// copy the signal into the AirFrame
	frame->setSignal(*s);

	// pointer and Signal not needed anymore
	delete s;
	s = 0;

	// TODO TEST: check if id is really unique
	frame->setId(world->getUniqueAirFrameId());

	ev << log("Creating TestAirFrame ") << i << " done." << endl;

	return frame;
}

// pass AirFrame-pointers currently on the (virtual) channel to SNRThresholdDecider
void DeciderTest::passAirFramesOnChannel(AirFrameVector& out)
{
	AirFrameVector::iterator it;

	// iterate over the member that holds the pointers to all AirFrames on
	// the virtual channel and put them to the output reference
	for (it = airFramesOnChannel.begin(); it != airFramesOnChannel.end(); it++)
	{
		out.push_back(*it);
	}

	ev << log("All TestAirFrames-pointers have been copied.") << endl;
}

/**
 * @brief Creates the right Signals for TestAirFrames (by index).
 *
 * By default TX-power and bitrate are constant for the whole duration of the Signal.
 *
 *
 */
Signal* DeciderTest::createSignal(simtime_t start,
									simtime_t length,
									std::pair<double, double> power,
									std::pair<double, double> bitrate,
									int index,
									simtime_t payloadStart = -1)
{
	simtime_t end = start + length;
	//create signal with start at current simtime and passed length
	Signal* s = new Signal(start, length);

	Mapping* txPowerMapping = 0;
	Mapping* bitrateMapping = 0;

	switch (index) {

		// TestAirFrame 5 has different TX-power for header and payload
		case 5:

			//set tx power mapping
			txPowerMapping = createHeaderPayloadMapping(start, payloadStart, end,
																	power.first, power.second);

			//set bitrate mapping
			bitrateMapping = createConstantMapping(start, end, bitrate.first);
			break;

		default:
			//set tx power mapping
			txPowerMapping = createConstantMapping(start, end, power.first);

			//set bitrate mapping
			bitrateMapping = createConstantMapping(start, end, bitrate.first);
			break;
	}

	assert(txPowerMapping);
	assert(bitrateMapping);

	// put the mappings to the Signal
	s->setTransmissionPower(txPowerMapping);
	s->setBitrate(bitrateMapping);

	ev << "Signal for TestAirFrame" << index << " created with header/payload power: " <<  power.first << "/" << power.second << endl;

	return s;
}

Mapping* DeciderTest::createConstantMapping(simtime_t start, simtime_t end, double value)
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

Mapping* DeciderTest::createHeaderPayloadMapping(	simtime_t start,
													simtime_t payloadStart,
													simtime_t end,
													double headerValue,
													double payloadValue)
{
	//create mapping over time
	Mapping* m = MappingUtils::createMapping(0.0, DimensionSet(Dimension::time), Mapping::LINEAR);

	//set position Argument
	Argument startPos(start);
	//set mapping at position
	m->setValue(startPos, headerValue);


	// header end (infinitely small time step earlier)
	Argument headerEndPos(pre(payloadStart));
	m->setValue(headerEndPos, headerValue);

	// payload start
	Argument payloadStartPos(payloadStart);
	m->setValue(payloadStartPos, payloadValue);


	//set position Argument
	Argument payloadEndPos(end);
	//set mapping at position
	m->setValue(payloadEndPos, payloadValue);

	return m;
}



// --- TESTING IMPLEMENTATION OF DeciderToPhyInterface !!! ---
// --- PLEASE REFER TO HEADER-FILE !!! ---

/**
 * SPECIAL TESTING IMPLEMENTATION: PLEASE REFER TO HEADER-FILE!
 */
void DeciderTest::getChannelInfo(simtime_t from, simtime_t to, AirFrameVector& out)
{

	switch (stateTestBDInitialization) {

		// there is no AirFrame on the Channel at the requested timepoint,
		// and we are not currently receiving an AirFrame
		case TEST_GET_CHANNELSTATE_EMPTYCHANNEL:
			// test whether values 'from' and 'to' are the same
			assertTrue( "Decider demands ChannelInfo for one single timepoint", from == to );
			assertEqual( "Decider demands ChannelInfo for current timepoint", getSimTime() , from);

			//  AirFrameVector referenced by 'out' remains as is
			break;

		// there are AirFrames on the Channel at the requested timepoint
		case TEST_GET_CHANNELSTATE_NOISYCHANNEL:

		//TODO this case must be considered separately, since SNRThresholdDecider will also ask for intervals
		case TEST_GET_CHANNELSTATE_RECEIVING:


		case TEST_SNR_THRESHOLD_ACCEPT:
		case TEST_SNR_THRESHOLD_DENY:
		case TEST_SNR_THRESHOLD_PAYLOAD_DENY:
		case TEST_SNR_THRESHOLD_MORE_NOISE_BEGINS_IN_BETWEEN_DENY:
		case TEST_SNR_THRESHOLD_NOISE_ENDS_AT_BEGINNING_DENY:
		case TEST_SNR_THRESHOLD_NOISE_BEGINS_AT_END_DENY:
		case TEST_CHANNELSENSE:

			if (processedAF != 0)
			{
				assert(testChannelSense == 0);

				const Signal& signal = processedAF->getSignal();

				// test whether Decider asks for the duration-interval of the processed AirFrame
				assertTrue( "Decider demands ChannelInfo for the duration of the AirFrame",
						(from == signal.getSignalStart()) &&
						(to == (signal.getSignalStart() + signal.getSignalLength())) );

				assertEqual( "Decider demands ChannelInfo for current test-timepoint (end of AirFrame)", getSimTime() , to);

			}
			else if(testChannelSense != 0)
			{
				assert(processedAF == 0);

				// test whether Decider asks for the duration-interval of the ChannelSense
				assertEqual( "Start of the interval which Decider requested is correct.",
							getSimTime() - testChannelSense->getSenseTimeout(),
							from);

				assertEqual( "End of the interval which Decider requested is correct.",
							getSimTime(),
							to);
			}
			else
			{
				// test whether values 'from' and 'to' are the same
				assertTrue( "Decider demands ChannelInfo for one single timepoint", from == to );
				assertEqual( "Decider demands ChannelInfo for current test-timepoint", getSimTime() , from);
			}

			// pass all AirFrames currently on the (virtual) channel
			passAirFramesOnChannel(out);

			break;


		// TODO go on here...

		default:
			break;
	}

	ev << log("All channel-info has been copied to AirFrameVector-reference.") << endl;

	return;
}

/**
 * SPECIAL TESTING IMPLEMENTATION: PLEASE REFER TO HEADER-FILE!
 */
void DeciderTest::sendControlMsg(cMessage* msg)
{

	ChannelSenseRequest* req = dynamic_cast<ChannelSenseRequest*>(msg);

	assertNotEqual("Control message send to mac by the Decider should be a ChannelSenseRequest", (void*)0, req);

	ChannelState state = req->getResult();

	assertEqual("ChannelSense results isIdle state match expected results isIdle state.",
				expChannelState.isIdle(), state.isIdle());
	assertEqual("ChannelSense results RSSI value match expected results RSSI value.",
				expChannelState.getRSSI(), state.getRSSI());

	// TODO as soon as ThresholdDecider sends 'packet dropped' - control messages
	// check correct sending of them here
}

/**
 * SPECIAL TESTING IMPLEMENTATION: PLEASE REFER TO HEADER-FILE!
 */
void DeciderTest::sendUp(AirFrame* packet, DeciderResult* result)
{

	// signal that method has been called (if testBaseDecider == true)
	sendUpCalled = true;

	switch (stateTestBDInitialization) {
		case TEST_GET_CHANNELSTATE_RECEIVING:
			// TODO check what the result should be in this test case
			// assertTrue("DeciderResult is: 'correct'", result.isSignalCorrect());
			assertTrue("BaseDecider has returned the pointer to currently processed AirFrame", (packet==processedAF));
			break;

		case TEST_SNR_THRESHOLD_ACCEPT:
			assertTrue("BaseDecider decided correctly to send up the packet", true);
			assertTrue("BaseDecider has returned the pointer to currently processed AirFrame", (packet==processedAF));
			break;

		default:
			break;
	}

	return;
}

/**
 * SPECIAL TESTING IMPLEMENTATION: PLEASE REFER TO HEADER-FILE!
 */
simtime_t DeciderTest::getSimTime()
{
	return testTime;
}

//
// --- implementation of SNRThresholdDecider-tests ---
//
void DeciderTest::doBaseDeciderTests()
{
	ChannelState cs;

	simtime_t nextHandoverTime = -1;


	switch (stateTestBDInitialization) {
		case TEST_GET_CHANNELSTATE_EMPTYCHANNEL:
		{
			// ask SNRThresholdDecider for the ChannelState, it should call getChannelInfo()
			// on the DeciderToPhyInterface
			testRSSIMap = MappingUtils::createMapping(0.0, DimensionSet(Dimension::time),
															Mapping::LINEAR);
			cs = decider->getChannelState();
			assertTrue("The PhyLayer is not currently receiving an AirFrame", cs.isIdle());
			assertEqual("RSSI-value is the value for 'now' in an empty Set of AirFrames (empty Mapping)",
					testRSSIMap->getValue(Argument(getSimTime())), cs.getRSSI());

			delete testRSSIMap;
			testRSSIMap = 0;
		}
			break;

		case TEST_GET_CHANNELSTATE_NOISYCHANNEL:
		{
			ev << log("-TEST_GET_CHANNELSTATE_NOISYCHANNEL-----------------------------------------------") << endl;


			// 1. testTime = before = t0
			testTime = before;

			// this method-call depends on the variable testTime,
			// so it needs to be set properly before
			fillAirFramesOnChannel();

			testRSSIMap = MappingUtils::createMapping(0.0, DimensionSet(Dimension::time),
														Mapping::LINEAR);
			// call getChannelState() of the SNRThresholdDecider
			cs = decider->getChannelState();
			assertTrue("The PhyLayer is not currently receiving an AirFrame", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime' in an empty Set of AirFrames (empty Mapping)",
						testRSSIMap->getValue(Argument(getSimTime())), cs.getRSSI());

			delete testRSSIMap;
			testRSSIMap = 0;

			ev << log("-full output-----------------------------------------------") << endl;

			// 2. testTime = t1
			testTime = t1;
			ev << log("A new testTime has been set: ")<< testTime << endl;

			ev << log("Putting AirFrames on the virtual channel.") << endl;
			fillAirFramesOnChannel();

			ev << log("Calling getChannelState() of BaseDecider.") << endl;
			cs = decider->getChannelState();

			ev << log("Checking results against expected results.") << endl;
			assertTrue("The PhyLayer is not currently receiving an AirFrame", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime'", res_t1_noisy, cs.getRSSI());

			ev << log("-short output-----------------------------------------------") << endl;

			// 3. testTime = t2
			testTime = t2;
			ev << log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();
			cs = decider->getChannelState();

			assertTrue("The PhyLayer is not currently receiving an AirFrame", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime'", res_t2_noisy, cs.getRSSI());

			ev << log("-short output-----------------------------------------------") << endl;

			// 4. testTime = t3
			testTime = t3;
			ev << log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();
			cs = decider->getChannelState();

			assertTrue("The PhyLayer is not currently receiving an AirFrame", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime'", res_t3_noisy, cs.getRSSI());

			ev << log("-short output-----------------------------------------------") << endl;

			// 5. testTime = t4
			testTime = t4;
			ev << log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();
			cs = decider->getChannelState();

			assertTrue("The PhyLayer is not currently receiving an AirFrame", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime'", res_t4_noisy, cs.getRSSI());

			ev << log("-short output-----------------------------------------------") << endl;

			// 6. testTime = t5
			testTime = t5;
			ev << log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();
			cs = decider->getChannelState();

			assertTrue("The PhyLayer is not currently receiving an AirFrame", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime'", res_t5_noisy, cs.getRSSI());

			ev << log("-short output-----------------------------------------------") << endl;

			// 7. testTime = t9
			testTime = t9;
			ev << log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();
			cs = decider->getChannelState();

			assertTrue("The PhyLayer is not currently receiving an AirFrame", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime'", res_t9_noisy, cs.getRSSI());

			ev << log("-DONE-------------------------------------------") << endl;

		}
			break;
		case TEST_GET_CHANNELSTATE_RECEIVING:
		{
			ev << log("-TEST_GET_CHANNELSTATE_RECEIVING-----------------------------------------------") << endl;

			ev << log("-full output-----------------------------------------------") << endl;

			// testTime = t2
			testTime = t2;
			ev << log("A new testTime has been set: ")<< testTime << endl;

			ev << log("Putting AirFrames on the virtual channel.") << endl;
			fillAirFramesOnChannel();

			ev << log("Calling getChannelState() of BaseDecider.") << endl;
			cs = decider->getChannelState();

			ev << log("Checking results against expected results.") << endl;
			assertTrue("The PhyLayer is not currently receiving an AirFrame", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime'", res_t2_receiving, cs.getRSSI());



			ev << log("-short output-----------------------------------------------") << endl;

			// testTime = t3
			testTime = t3;
			ev << log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();

			// by now we are not yet receiving something
			cs = decider->getChannelState();
			assertTrue("The PhyLayer is not currently receiving an AirFrame", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime'", res_t3_receiving_before, cs.getRSSI());

			// trying to receive an AirFrame whose signal (TXpower) is too weak
			ev << log("Trying to receive an AirFrame whose TXpower is too low.") << endl;
			nextHandoverTime = decider->processSignal(TestAF2);
			assertTrue("AirFrame has been rejected, since TXpower is too low.", (nextHandoverTime < 0));

			// starting to receive TestAirFrame 3
			ev << log("Trying to receive TestAirFrame 3") << endl;
			nextHandoverTime = decider->processSignal(TestAF3);
			Signal& signal3 = TestAF3->getSignal();
			assertTrue("TestAirFrame 3 can be received, end-time is returned",
					(nextHandoverTime == signal3.getSignalStart() + signal3.getSignalLength()));


			// try to receive another AirFrame at the same time, whose signal not too weak
			// (taking a copy of the currently received one)
			AirFrame* tempAF = new AirFrame(*TestAF3);
			ev << log("Trying to receive another AirFrame at the same time.") << endl;
			nextHandoverTime = decider->processSignal(tempAF);
			assertTrue("AirFrame has been rejected, since we already receive one.", (nextHandoverTime < 0));
			delete tempAF;
			tempAF = 0;


			// now we have started to receive an AirFrame
			cs = decider->getChannelState();
			assertTrue("The PhyLayer IS currently receiving an AirFrame", !(cs.isIdle()));
			assertEqual("RSSI-value is the value for 'testTime'", res_t3_receiving_after, cs.getRSSI());


			ev << log("-short output-----------------------------------------------") << endl;

			// testTime = t4
			testTime = t4;
			ev << log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();

			// now we have started to receive an AirFrame
			cs = decider->getChannelState();
			assertTrue("The PhyLayer IS currently receiving an AirFrame", !(cs.isIdle()));
			assertEqual("RSSI-value is the value for 'testTime'", res_t4_receiving, cs.getRSSI());


			ev << log("-short output-----------------------------------------------") << endl;

			// testTime = t5
			testTime = t5;
			ev << log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();

			// now we have started to receive an AirFrame
			cs = decider->getChannelState();
			assertTrue("The PhyLayer IS currently receiving an AirFrame", !(cs.isIdle()));
			assertEqual("RSSI-value is the value for 'testTime'", res_t5_receiving_before, cs.getRSSI());

			//hand over the AirFrame again, end-time is reached
			ev << log("Handing over TestAirFrame 3 again, transmission has finished") << endl;

			// set the pointer to the processed AirFrame before SNRThresholdDecider will finally process it
			processedAF = TestAF3;
			nextHandoverTime = decider->processSignal(TestAF3);
			assertTrue("TestAirFrame 3 has been finally processed",
					(nextHandoverTime < 0));
			processedAF = 0;

			// now receive of the AirFrame is over
			cs = decider->getChannelState();
			assertTrue("The PhyLayer is not currently receiving an AirFrame", cs.isIdle());
			assertEqual("RSSI-value is the value for 'testTime'", res_t5_receiving_after, cs.getRSSI());

			// SNRThresholdDecider should be able to handle the next AirFrame
			ev << log("Trying to immediately receive TestAirFrame 4") << endl;
			nextHandoverTime = decider->processSignal(TestAF4);
			Signal& signal4 = TestAF4->getSignal();
			assertTrue("TestAirFrame 4 can be received, end-time is returned",
					(nextHandoverTime == signal4.getSignalStart() + signal4.getSignalLength()));



			ev << log("-short output-----------------------------------------------") << endl;

			// testTime = t6
			testTime = t6;
			ev << log("A new testTime has been set: ")<< testTime << endl;

			fillAirFramesOnChannel();

			// now we are not receiving an AirFrame
			cs = decider->getChannelState();
			assertTrue("The PhyLayer is again receiving an AirFrame", !(cs.isIdle()));
			assertEqual("RSSI-value is the value for 'testTime'", res_t6_receiving, cs.getRSSI());

			// set the pointer to the processed AirFrame before SNRThresholdDecider will finally process it
			processedAF = TestAF4;
			nextHandoverTime = decider->processSignal(TestAF4);
			assertTrue("TestAirFrame 4 has been finally processed",
					(nextHandoverTime < 0));
			processedAF = 0;

			ev << log("-DONE-------------------------------------------") << endl;



		}
			break;
//		case TEST_SNR_THRESHOLD:
//		{
//			// TODO insert test cases
//
//			ev << log("-TEST_SNR_THRESHOLD-----------------------------------------------") << endl;
//
//			//TODO this is just an example, not a real test
//			testTime = t1;
//			fillAirFramesOnChannel();
//
//			processedAF = TestAF5;
//			nextHandoverTime = decider->processSignal(TestAF5);
//			Signal& signal5 = TestAF5->getSignal();
//			assertTrue("TestAirFrame 5 can be received, end-time is returned",
//					(nextHandoverTime == signal5.getSignalStart() + signal5.getSignalLength()));
//
//
//			assertEqual("NextHandoverTime is t5.", t5, nextHandoverTime);
//			testTime = nextHandoverTime;
//			fillAirFramesOnChannel();
//
//			nextHandoverTime = decider->processSignal(TestAF5);
//
//			assertTrue("TestAirFrame 5 has been finally processed",
//								(nextHandoverTime < 0));
//			processedAF = 0;
//
//			ev << log("-DONE-------------------------------------------") << endl;
//		}
//			break;

		// here we test a simple case where one noise-AirFrame is present, but SNR is just high enough
		case TEST_SNR_THRESHOLD_ACCEPT:

			ev << log("-------------------------------------------------------") << endl;
			ev << log(stateToString(stateTestBDInitialization)) << endl;

			testTime = t1;
			fillAirFramesOnChannel();

			sendUpCalled = false;

			processedAF = TestAF5;
			nextHandoverTime = decider->processSignal(TestAF5);

			assertEqual("NextHandoverTime is t5.", t5, nextHandoverTime);

			testTime = nextHandoverTime;

			nextHandoverTime = decider->processSignal(TestAF5);

			assertTrue("TestAirFrame 5 has been finally processed",
								(nextHandoverTime < 0));
			processedAF = 0;

			assertTrue("sendUp() has been called.", sendUpCalled);

			break;


		// here we test a simple case where one noise-AirFrame is present, but SNR is just not high enough
		case TEST_SNR_THRESHOLD_DENY:

		// here we test a simple case where one noise-AirFrame is present and  header-SNR is just high enough
		// but payload-SNR is just not high enough
		case TEST_SNR_THRESHOLD_PAYLOAD_DENY:

		// here we test a simple case where a second noise-AirFrame arrives during reception of an AirFrame
		// and thus SNR-level turns too low
		case TEST_SNR_THRESHOLD_MORE_NOISE_BEGINS_IN_BETWEEN_DENY:

		// here we test a simple case where one noise-AirFrame ends at the beginning of the reception
		// thus SNR-level is too low
		case TEST_SNR_THRESHOLD_NOISE_ENDS_AT_BEGINNING_DENY:

		// here we test a simple case where one noise-AirFrame begins at the end of the reception
		// thus SNR-level is too low
		case TEST_SNR_THRESHOLD_NOISE_BEGINS_AT_END_DENY:

			ev << log("-------------------------------------------------------") << endl;
			ev << log(stateToString(stateTestBDInitialization)) << endl;

			testTime = t1;
			fillAirFramesOnChannel();

			sendUpCalled = false;

			processedAF = TestAF5;
			nextHandoverTime = decider->processSignal(TestAF5);

			assertEqual("NextHandoverTime is t5.", t5, nextHandoverTime);

			testTime = nextHandoverTime;

			nextHandoverTime = decider->processSignal(TestAF5);

			assertTrue("TestAirFrame 5 has been finally processed",
								(nextHandoverTime < 0));
			processedAF = 0;

			assertFalse("sendUp() has not been called.", sendUpCalled);

			break;

		case TEST_CHANNELSENSE:
			processedAF = 0;

			ev << log("-------------------------------------------------------") << endl;
			ev << log(stateToString(stateTestBDInitialization)) << " - empty channel"<< endl;
			//test sense on empty channel
			testTime = before;
			fillAirFramesOnChannel();
			testChannelSense = new ChannelSenseRequest();
			testChannelSense->setSenseTimeout(0.2);
			testChannelSense->setSenseMode(UNTIL_TIMEOUT);

			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);

			assertEqual("NextHandoverTime is correct current time + sense duration.",
						testTime + 0.2, nextHandoverTime);
			testTime = nextHandoverTime;

			expChannelState = ChannelState(true, 0.0);
			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);
			assertTrue("NextHandoverTime express that sense request has been finally processed.",
					   nextHandoverTime < 0);

			delete testChannelSense;


			//test sense during one single airframe without an AirFrame receiving
			ev << log("-------------------------------------------------------") << endl;
			ev << log(stateToString(stateTestBDInitialization)) << " - signel airframe on channel"<< endl;
			testTime = t1;
			fillAirFramesOnChannel();
			testChannelSense = new ChannelSenseRequest();
			testChannelSense->setSenseTimeout(0.2);
			testChannelSense->setSenseMode(UNTIL_TIMEOUT);

			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);

			assertEqual("NextHandoverTime is correct current time + sense duration.",
						testTime + 0.2, nextHandoverTime);
			testTime = nextHandoverTime;

			expChannelState = ChannelState(true, TXpower1);
			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);
			assertTrue("NextHandoverTime express that sense request has been finally processed.",
					   nextHandoverTime < 0);

			delete testChannelSense;

			//test sense with new AirFrames at end of sense without an AirFrame receiving
			ev << log("-------------------------------------------------------") << endl;
			ev << log(stateToString(stateTestBDInitialization)) << " - new airframes at end of sense"<< endl;
			testTime = t2;
			fillAirFramesOnChannel();
			testChannelSense = new ChannelSenseRequest();
			testChannelSense->setSenseTimeout(1.0);
			testChannelSense->setSenseMode(UNTIL_TIMEOUT);

			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);

			assertEqual("NextHandoverTime is correct current time + sense duration.",
						testTime + 1.0, nextHandoverTime);
			testTime = nextHandoverTime;
			fillAirFramesOnChannel();

			expChannelState = ChannelState(true, TXpower1 + TXpower2 + TXpower3);
			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);
			assertTrue("NextHandoverTime express that sense request has been finally processed.",
					   nextHandoverTime < 0);

			delete testChannelSense;

			//test sense with new AirFrames during sense without an AirFrame receiving
			ev << log("-------------------------------------------------------") << endl;
			ev << log(stateToString(stateTestBDInitialization)) << " - new AirFrames during sense"<< endl;
			testTime = t2;
			fillAirFramesOnChannel();
			testChannelSense = new ChannelSenseRequest();
			testChannelSense->setSenseTimeout(2.0);
			testChannelSense->setSenseMode(UNTIL_TIMEOUT);

			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);

			assertEqual("NextHandoverTime is correct current time + sense duration.",
						testTime + 2.0, nextHandoverTime);
			testTime = nextHandoverTime;
			fillAirFramesOnChannel();

			expChannelState = ChannelState(true, TXpower1 + TXpower2 + TXpower3);
			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);
			assertTrue("NextHandoverTime express that sense request has been finally processed.",
					   nextHandoverTime < 0);

			delete testChannelSense;

			//test sense with AirFrames end at start of sense without an AirFrame receiving
			ev << log("-------------------------------------------------------") << endl;
			ev << log(stateToString(stateTestBDInitialization)) << " - AirFrames ends at start of sense"<< endl;
			testTime = t7;
			fillAirFramesOnChannel();
			testChannelSense = new ChannelSenseRequest();
			testChannelSense->setSenseTimeout(1.0);
			testChannelSense->setSenseMode(UNTIL_TIMEOUT);

			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);

			assertEqual("NextHandoverTime is correct current time + sense duration.",
						testTime + 1.0, nextHandoverTime);
			testTime = nextHandoverTime;

			expChannelState = ChannelState(true, TXpower1 + TXpower2);
			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);
			assertTrue("NextHandoverTime express that sense request has been finally processed.",
					   nextHandoverTime < 0);

			delete testChannelSense;

			//test sense with new AirFrames during sense with an AirFrame receiving
			ev << log("-------------------------------------------------------") << endl;
			ev << log(stateToString(stateTestBDInitialization)) << " - new AirFrames during sense not idle"<< endl;
			testTime = t2;
			fillAirFramesOnChannel();
			testChannelSense = new ChannelSenseRequest();
			testChannelSense->setSenseTimeout(2.0);
			testChannelSense->setSenseMode(UNTIL_TIMEOUT);

			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);

			assertEqual("NextHandoverTime is correct current time + sense duration.",
						testTime + 2.0, nextHandoverTime);

			testTime = t3;
			fillAirFramesOnChannel();
			nextHandoverTime = decider->processSignal(TestAF3);
			assertTrue("TestAF3 should be received by decider.", nextHandoverTime > 0.0);

			testTime = t4;
			fillAirFramesOnChannel();

			expChannelState = ChannelState(false, TXpower1 + TXpower2 + TXpower3);
			nextHandoverTime = decider->handleChannelSenseRequest(testChannelSense);
			assertTrue("NextHandoverTime express that sense request has been finally processed.",
					   nextHandoverTime < 0);

			delete testChannelSense;
			break;

		default:
			break;
	}
}

/**
 * Return a string with the pattern
 * "[module name] - passed text"
 */
std::string DeciderTest::log(std::string msg) {
	// TODO the name in the output currently is the name of the former module
	// this avoids unnecessary diffs, change name when expected output has been updated
	return "[phy0] - " + msg;
}


//TODO implement
ConstMapping* DeciderTest::getThermalNoise(simtime_t from, simtime_t to)
{
	return 0;
}

// TODO implement
void DeciderTest::cancelScheduledMessage(cMessage* msg)
{
	return;
}

// TODO implement
void DeciderTest::drawCurrent(double amount, int activity)
{
	return;
}

// TODO implement
BaseUtility* DeciderTest::getUtility()
{
	return 0;
}

// TODO implement
BaseWorldUtility* DeciderTest::getWorldUtility()
{
	return 0;
}

// TODO implement
void DeciderTest::recordScalar(const char *name, double value, const char *unit)
{
	return;
}


void DeciderTest::runTests()
{
	// former initialization (stage == 1)

	// formerly done in BasePhyLayers
	// TODO: to test more than one Decider, here must be passed an argument
	// dummy parameter
	cXMLElement* dummy = 0;
	initializeDecider(dummy);

	// start the test of the decider
	testBaseDeciderInitialization();
}

void DeciderTest::initializeDecider(cXMLElement* xmlConfig)
{
	// dummy parameters
	std::string dummyName;
	ParameterMap dummyParams;

	decider = getDeciderFromName(dummyName, dummyParams);
}
