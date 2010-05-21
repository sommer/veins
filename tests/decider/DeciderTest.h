#ifndef TESTPHYLAYER_H_
#define TESTPHYLAYER_H_

#include <DeciderToPhyInterface.h>
#include <MacToPhyInterface.h>
#include <OmnetTestBase.h>
#include "TestBaseDecider.h"

#include <list>
//#include <utility>


class DeciderTest : public DeciderToPhyInterface, public OmnetTestBase {
private:

protected:


	int myIndex;

	Decider* decider;

	// Indicator, whether a TestBaseDecider is used as Decider
	bool testBaseDecider;

	// prepared RSSI mapping for testing purposes
	Mapping* testRSSIMap;

	// member to store AirFrame on the channel (for current testing state)
	DeciderToPhyInterface::AirFrameVector airFramesOnChannel;

	/**
	 * @brief Used at initialisation to pass the parameters
	 * to the AnalogueModel and Decider
	 */
	typedef std::map<std::string, cMsgPar> ParameterMap;

	/**
	 * @brief Small service-class.
	 */
	class TestWorld
	{
	protected:

		/** @brief Provides a unique number for AirFrames per simulation */
		long airFrameId;

	public:

		TestWorld()
		{
			airFrameId = 0;
		}

		~TestWorld() {}

		/** @brief Returns an Id for an AirFrame, at the moment simply an incremented long-value */
	    long getUniqueAirFrameId(){

	    	// if counter has done one complete cycle and will be set to a value it already had
	    	if (airFrameId == -1){
	    		// print a warning
	    		ev << "WARNING: AirFrameId-Counter has done one complete cycle."
	    		<< " AirFrameIds are repeating now and may not be unique anymore." << endl;
	    	}

	    	return airFrameId++;
	    }
	};

	/**
	 * @ brief Method to initialize Decider to be tested.
	 */
	void initializeDecider(cXMLElement* xmlConfig);


	// list containing simtime-to-double entries
	// typedef std::list<std::pair<simtime_t, double> > KeyList;

	virtual Decider* getDeciderFromName(std::string name, ParameterMap& params);

	void testBaseDeciderInitialization();

	void doBaseDeciderTests();



	enum
	{
		BEFORE_TESTS = 0,

		TEST_GET_CHANNELSTATE_EMPTYCHANNEL = 100,

		TEST_GET_CHANNELSTATE_NOISYCHANNEL,
		TEST_GET_CHANNELSTATE_RECEIVING,


		TEST_SNR_THRESHOLD_ACCEPT,
		TEST_SNR_THRESHOLD_DENY,
		TEST_SNR_THRESHOLD_PAYLOAD_DENY,
		TEST_SNR_THRESHOLD_MORE_NOISE_BEGINS_IN_BETWEEN_DENY,
		TEST_SNR_THRESHOLD_NOISE_ENDS_AT_BEGINNING_DENY,
		TEST_SNR_THRESHOLD_NOISE_BEGINS_AT_END_DENY,

		TEST_CHANNELSENSE,

		SIMULATION_RUN = 1000

	} stateTestBDInitialization;


	std::string stateToString(int state)
	{
		switch (state) {
			case BEFORE_TESTS:
				return "BEFORE_TESTS";

			case TEST_GET_CHANNELSTATE_EMPTYCHANNEL:
				return "TEST_GET_CHANNELSTATE_EMPTYCHANNEL";

			case TEST_GET_CHANNELSTATE_NOISYCHANNEL:
				return "TEST_GET_CHANNELSTATE_NOISYCHANNEL";
			case TEST_GET_CHANNELSTATE_RECEIVING:
				return "TEST_GET_CHANNELSTATE_RECEIVING";


			case TEST_SNR_THRESHOLD_ACCEPT:
				return "TEST_SNR_THRESHOLD_ACCEPT";
			case TEST_SNR_THRESHOLD_DENY:
				return "TEST_SNR_THRESHOLD_DENY";
			case TEST_SNR_THRESHOLD_PAYLOAD_DENY:
				return "TEST_SNR_THRESHOLD_PAYLOAD_DENY";
			case TEST_SNR_THRESHOLD_MORE_NOISE_BEGINS_IN_BETWEEN_DENY:
				return "TEST_SNR_THRESHOLD_MORE_NOISE_BEGINS_IN_BETWEEN_DENY";
			case TEST_SNR_THRESHOLD_NOISE_ENDS_AT_BEGINNING_DENY:
				return "TEST_SNR_THRESHOLD_NOISE_ENDS_AT_BEGINNING_DENY";
			case TEST_SNR_THRESHOLD_NOISE_BEGINS_AT_END_DENY:
				return "TEST_SNR_THRESHOLD_NOISE_BEGINS_AT_END_DENY";

			case TEST_CHANNELSENSE:
				return "TEST_CHANNELSENSE";

			case SIMULATION_RUN:
				return "SIMULATION_RUN";

			default:
				assertFalse("Correct state found.", true);
				return "Unknown state.";
		}

	}

	// method to fill the member airFramesOnChannel with AirFrames
	// according to testing state
	void fillAirFramesOnChannel();

	// create a test AirFrame identified by an index
	AirFrame* createTestAirFrame(int i);

	// pass AirFrames currently on the (virtual) channel to SNRThresholdDecider
	void passAirFramesOnChannel(AirFrameVector& out);

	/**
	 * @brief Creates a simple Signal defined over time with the
	 * passed parameters.
	 *
	 * Signals that are not constant over the whole duration can also be
	 * created.
	 *
	 * Convenience method to be able to create the appropriate
	 * Signal for the MacToPhyControlInfo without needing to care
	 * about creating Mappings.
	 */
	virtual Signal* createSignal(simtime_t start,
									simtime_t length,
									std::pair<double, double> power,
									std::pair<double, double> bitrate,
									int index,
									simtime_t payloadStart);

	/**
	 * @brief Creates a simple Mapping with a constant curve
	 * progression at the passed value.
	 *
	 * Used by "createSignal" to create the power and bitrate mapping.
	 */
	Mapping* createConstantMapping(simtime_t start, simtime_t end, double value);

	/**
	 * @brief Creates a quasi-step mapping containing all entries of the passed list,
	 * using linear interpolation of mappings.
	 * (i.e. creates additional entries in the mapping as close as possible to the key-entries)
	 *
	 * NOTE:
	 * The first and the last element of the passed list are considered start- and end-point of the mapping.
	 *
	 * Used by "createSignal" to create the power and bitrate mapping.
	 */
	 // Mapping* createVariableMapping(const KeyList& list);


	/**
	 * @brief Creates a mapping with separate values for header and payload of an AirFrame.
	 *
	 * This is a 'quasi-step' mapping, using a linear interpolated mapping, i.e. a
	 * step is realized by two succeeding key-entries in the mapping with linear
	 * interpolation between them.
	 *
	 * Used by "createSignal" to create the power and bitrate mapping.
	 *
	 */
	Mapping* createHeaderPayloadMapping(simtime_t start,
										simtime_t payloadStart,
										simtime_t end,
										double headerValue,
										double payloadValue);



	// NOTE: The following members are for testing the SNRThresholdDecider
	// so they are only initialized if testBaseDecider==true
	// see also: DeciderTest::initialize()

	// some fix time-points for the signals
	simtime_t t0;
	simtime_t t1;
	simtime_t t3;
	simtime_t t5;
	simtime_t t7;
	simtime_t t9;

	// time-points before and after
	simtime_t before;
	simtime_t after;

	// time-points in between
	simtime_t t2;
	simtime_t t4;
	simtime_t t6;
	simtime_t t8;

	simtime_t testTime;

	// some test AirFrames
	AirFrame* TestAF1;
	AirFrame* TestAF2;
	AirFrame* TestAF3;
	AirFrame* TestAF4;

	// AirFrames for SNR-threshold tests
	AirFrame* TestAF5;
	AirFrame* TestAF6;


	// Used for ChannelSenseRequest tests to define the expected result of a Test sense
	ChannelState expChannelState;

	// Represents the currently used ChannelSenseRequest which is tested
	ChannelSenseRequest*  testChannelSense;

	// pointer to the AirFrame that is currently processed by SNRThresholdDecider
	const AirFrame* processedAF;

	// value for no attenuation (in attenuation-mappings)
	double noAttenuation;


	// some TX-power values
	double TXpower1;
	double TXpower2;
	double TXpower3;
	double TXpower4;
	double TXpower5P;
	double TXpower5H;
	double TXpower6;

	// some bitrates
	double bitrate9600;

	// expected results of tests (hard coded)
	double res_t1_noisy;
	double res_t2_noisy;
	double res_t3_noisy;
	double res_t4_noisy;
	double res_t5_noisy;
	double res_t9_noisy;

	double res_t2_receiving;
	double res_t3_receiving_before;
	double res_t3_receiving_after;
	double res_t4_receiving;
	double res_t5_receiving_before;
	double res_t5_receiving_after;
	double res_t6_receiving;

	// flag to signal whether sendUp() has been called by SNRThresholdDecider
	bool sendUpCalled;

	// dummy move
	Move move;

	// minimal world for testing purposes
	TestWorld* world;


	// TODO test whether this construction of the smallest possible time step works)
	// TODO implement
	/**
	 * @brief returns the closest value of simtime before passed value
	 *
	 * Works only for arguments t > 0;
	 */
	simtime_t pre(simtime_t t)
	{
		if (t.raw() > 0)
			t.setRaw(t.raw() - 1);

		return t;
	}

public:
	DeciderTest();

	virtual ~DeciderTest();


	//---------DeciderToPhyInterface implementation-----------
	// Taken from BasePhyLayer. Will be overridden to control
	// BaseDeciders calls on the Interface during testBaseDeciderInitialization().
	//
	// Depending on whether we are still in testBaseDeciderInitialization() of DeciderTest (*)
	// or the simulation is already running and real AirFrames are sent (**), either BaseDeciders
	// calls are caught and special values are returned (*) or the original implementation of
	// the super-class ( BasePhyLayer::_______ ) is called and everything is as usual (**).

	/**
	 * @brief Fills the passed AirFrameVector with all AirFrames that intersect
	 * with the time interval [from, to]
	 */
	virtual void getChannelInfo(simtime_t from, simtime_t to, AirFrameVector& out);

	/**
	 * @brief Called by the Decider to send a control message to the MACLayer
	 *
	 * This function can be used to answer a ChannelSenseRequest to the MACLayer
	 *
	 */
	virtual void sendControlMsg(cMessage* msg);

	/**
	 * @brief Called to send an AirFrame with DeciderResult to the MACLayer
	 *
	 * When a packet is completely received and not noise, the Decider
	 * call this function to send the packet together with
	 * the corresponding DeciderResult up to MACLayer
	 *
	 */
	virtual void sendUp(AirFrame* packet, DeciderResult* result);

	/**
	 * @brief Returns the current simulation time or a special test-time
	 *
	 */
	virtual simtime_t getSimTime();

	/**
	 * Return a string with the pattern
	 * "[module name] - passed text"
	 */
	std::string log(std::string msg);

	// TODO: implement the following methods

	// TODO think of whether thermal noise is important for testing
	/**
	 * @brief Returns a Mapping which defines the thermal noise in
	 * the passed time frame (in mW).
	 *
	 * The implementing class of this method keeps ownership of the
	 * Mapping.
	 */
	virtual ConstMapping* getThermalNoise(simtime_t from, simtime_t to);

	/**
	 * @brief Tells the PhyLayer to cancel a scheduled message (AirFrame or
	 * ControlMessage).
	 *
	 * Used by the Decider if it has to handle an AirFrame or a control message
	 * earlier than it has returned to the PhyLayer the last time the Decider
	 * handled that message.
	 */
	virtual void cancelScheduledMessage(cMessage* msg);

	/**
	 * @brief Enables the Decider to draw Power from the
	 * phy layers power accounts.
	 *
	 * Does nothing if no Battery module in simulation is present.
	 */
	virtual void drawCurrent(double amount, int activity);

	/**
	 * @brief Returns a pointer to the hosts utility-module.
	 */
	virtual BaseUtility* getUtility();

	/**
	 * @brief Returns a pointer to the simulations world-utility-module.
	 */
	virtual BaseWorldUtility* getWorldUtility();

	/**
	 * @brief Utility method to enable a Decider, which isn't an OMNeT-module, to
	 * use the OMNeT-method 'recordScalar' with the help of and through its interface to BasePhyLayer.
	 *
	 * The method-signature is taken from OMNeTs 'ccomponent.h' but made pure virtual here.
	 * See the original method-description below:
	 *
	 * Records a double into the scalar result file.
	 */
	virtual void recordScalar(const char *name, double value, const char *unit=NULL);


	//---------OmnetTestBase implementation-----------

	virtual void runTests();
};

#endif /*TESTPHYLAYER_H_*/
