#ifndef TESTPHYLAYER_H_
#define TESTPHYLAYER_H_

#include <BasePhyLayer.h>
#include <TestModule.h>
#include "TestDecider.h"
#include "TestBaseDecider.h"


class TestPhyLayer:public BasePhyLayer, public TestModule {
private:

	class TestAnalogueModel:public AnalogueModel {
	public:
		double att;

		TestAnalogueModel(double attenuation):
			att(attenuation) {}

		void filterSignal(Signal& s) {
			return;
		}
	};
protected:
	int myIndex;

	// Indicator, whether a TestBaseDecider is used as Decider
	bool testBaseDecider;

	// prepared RSSI mapping for testing purposes
	Mapping* testRSSIMap;

	// member to store AirFrame on the channel (for current testing state)
	DeciderToPhyInterface::AirFrameVector airFramesOnChannel;

	virtual AnalogueModel* getAnalogueModelFromName(std::string name, ParameterMap& params);

	virtual Decider* getDeciderFromName(std::string name, ParameterMap& params);

	void testInitialisation();

	void testBaseDeciderInitialization();

	void doBaseDeciderTests();

	enum
	{
		BEFORE_TESTS = 0,

		TEST_GET_CHANNELSTATE_EMPTYCHANNEL = 100,
		TEST_GET_CHANNELSTATE_NOISYCHANNEL,

		SIMULATION_RUN = 1000

	} stateTestBDInitialization;

	// method to fill the member airFramesOnChannel with AirFrames
	// according to testing state
	void fillAirFramesOnChannel();

	// create a test AirFrame identified by an index
	AirFrame* createTestAirFrame(int i);

	// pass AirFrames currently on the (virtual) channel to BaseDecider
	void passAirFramesOnChannel(AirFrameVector& out);

	/**
	 * @brief Creates a simple Signal defined over time with the
	 * passed parameters.
	 *
	 * Convenience method to be able to create the appropriate
	 * Signal for the MacToPhyControlInfo without needing to care
	 * about creating Mappings.
	 */
	virtual Signal* createSignal(simtime_t start, simtime_t length, double power, double bitrate);

	/**
	 * @brief Creates a simple Mapping with a constant curve
	 * progression at the passed value.
	 *
	 * Used by "createSignal" to create the power and bitrate mapping.
	 */
	Mapping* createConstantMapping(simtime_t start, simtime_t end, double value);



	// NOTE: The following members are for testing the BaseDecider
	// so they are only initialized if testBaseDecider==true
	// see also: TestPhyLayer::initialize()

	// some fix time-points for the signals
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

	// value for no attenuation (in attenuation-mappings)
	double noAttenuation;


	// some TX-power values
	double TXpower1;
	double TXpower2;
	double TXpower3;

	// some bitrates
	double bitrate9600;

	// expected results of tests (hard coded)
	double res_t1_noisy;
	double res_t2_noisy;
	double res_t3_noisy;
	double res_t4_noisy;
	double res_t5_noisy;
	double res_t9_noisy;


public:
	virtual void initialize(int stage);

	virtual void handleMessage(cMessage* msg);

	virtual ~TestPhyLayer();

	// convenience method for the Mac-Layer
	virtual bool useBaseDecider()
	{
		return testBaseDecider;
	}

	//---------DeciderToPhyInterface implementation-----------
	// Taken from BasePhyLayer. Will be overridden to control
	// BaseDeciders calls on the Interface during testBaseDeciderInitialization().
	//
	// Depending on whether we are still in testBaseDeciderInitialization() of TestPhyLayer (*)
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
	virtual void sendUp(AirFrame* packet, DeciderResult result);

	/**
	 * @brief Returns the current simulation time or a special test-time
	 *
	 */
	virtual simtime_t getSimTime();
};

#endif /*TESTPHYLAYER_H_*/
