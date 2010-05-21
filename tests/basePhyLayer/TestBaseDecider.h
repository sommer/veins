#ifndef TESTBASEDECIDER_H_
#define TESTBASEDECIDER_H_

#include <SNRThresholdDecider.h>
#include <TestModule.h>
#include <BasePhyLayer.h>
#include <cassert>


#include "TestGlobals.h"

class TestBaseDecider : public SNRThresholdDecider, public TestModule
{

protected:

protected:

	void assertMessage(std::string msg, int state, AirFrame* frame, simtime_t arrival, std::string dest = "")
	{
			TestModule::assertMessage(new AssertAirFrame(msg, state, arrival, frame), dest);
	}

	void checkInitMembers(DeciderToPhyInterface* phy, double threshold,	double sensitivity)
	{
		/* check whether... */

		// pointer to DeciderToPhyInterface has been set properly
		assertEqual(TestModule::log("pointer to DeciderToPhyInterface has been set properly"), this->phy, phy);

		// snr-threshold and receiving-sensitivity have been set properly
		assertTrue(TestModule::log("snr-threshold and receiving-sensitivity have been set properly"),
				((this->snrThreshold == threshold) && (this->sensitivity == sensitivity)) );
	}



public:

	TestBaseDecider(DeciderToPhyInterface* phy,
					double threshold,
					double sensitivity,
					int myIndex,
					bool debug)
		: SNRThresholdDecider(phy, threshold, sensitivity, myIndex, debug)
	{

		init("decider" + toString(this->myIndex));
		assert(phy);

		// check if member 'myIndex' has been set properly
		assertEqual(TestModule::log("Member 'myIndex' has been initialized properly by passed value."),
				this->myIndex, myIndex);

		// check if everything else has been set properly
		checkInitMembers(phy, threshold, sensitivity);
	}

	TestBaseDecider(DeciderToPhyInterface* phy,
					double threshold,
					double sensitivity)
		: SNRThresholdDecider(phy, threshold, sensitivity)
	{

		// TODO: here we have the problem, that multiple deciders might
		// have the same default 'myIndex'-value if not passed in constructor call
		init("decider" + toString(this->myIndex));
		assert(phy);

		// check if member 'myIndex' has been set properly
		assertEqual(TestModule::log("Member 'myIndex' has been initialized properly by default."),
				this->myIndex, -1);

		// check if everything else has been set properly
		checkInitMembers(phy, threshold, sensitivity);
	}

	virtual ~TestBaseDecider()
	{
		finalize();
	}


	/*
	 * Here are the overridden public methods of SNRThresholdDecider.
	 *
	 * Tests can be made and the method of SNRThresholdDecider (the one that shall be tested)
	 * is called.
	 *
	 */
	// TODO: finish

	/**
	 * @brief Just a bypass at the moment.
	 */
	virtual ChannelState getChannelState()
	{
		return SNRThresholdDecider::getChannelState();
	}

	/**
	 * @brief Just a bypass at the moment.
	 */
	virtual simtime_t processSignal(AirFrame* frame)
	{
		return SNRThresholdDecider::processSignal(frame);
	}


	// TODO add the others
};


#endif /* TESTBASEDECIDER_H_ */
