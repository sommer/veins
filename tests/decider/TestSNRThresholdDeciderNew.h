#ifndef TESTSNRTHRESHOLDDECIDERNEW_H_
#define TESTSNRTHRESHOLDDECIDERNEW_H_

#include <SNRThresholdDecider.h>
#include <BasePhyLayer.h>
#include <cassert>

class TestSNRThresholdDeciderNew : public SNRThresholdDecider
{
protected:
	/**
	 * Return a string with the pattern
	 * "[module name] - passed text"
	 */
	std::string log(std::string msg) {
		return "[TestBaseDecider] - " + msg;
	}

	void checkInitMembers(DeciderToPhyInterface* phy, double threshold,	double sensitivity)
	{
		/* check whether... */

		// pointer to DeciderToPhyInterface has been set properly
		assertEqual(log("pointer to DeciderToPhyInterface has been set properly"), this->phy, phy);

		// snr-threshold and receiving-sensitivity have been set properly
		assertTrue(log("snr-threshold and receiving-sensitivity have been set properly"),
				((this->snrThreshold == threshold) && (this->sensitivity == sensitivity)) );
	}



public:

	TestSNRThresholdDeciderNew(DeciderToPhyInterface* phy,
					double snrThreshold,
					double sensitivity,
					double busyThreshold,
					int myIndex,
					bool debug)
		: SNRThresholdDecider(phy, snrThreshold, sensitivity, busyThreshold, myIndex, debug)
	{

		// commented out, because TestBaseLayer no longer subclasses TestModule
		//init("decider" + toString(this->myIndex));
		assert(phy);

		// check if member 'myIndex' has been set properly
		assertEqual(log("Member 'myIndex' has been initialized properly by passed value."),
				this->myIndex, myIndex);

		// check if everything else has been set properly
		checkInitMembers(phy, snrThreshold, sensitivity);
	}
};


#endif /* TESTSNRTHRESHOLDDECIDERNEW_H_ */
