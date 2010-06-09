#ifndef TESTSNRTHRESHOLDDECIDER_H_
#define TESTSNRTHRESHOLDDECIDER_H_

#include <SNRThresholdDeciderOld.h>
#include <BasePhyLayer.h>
#include <cassert>

class TestSNRThresholdDecider : public SNRThresholdDeciderOld
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

	TestSNRThresholdDecider(DeciderToPhyInterface* phy,
					double threshold,
					double sensitivity,
					int myIndex,
					bool debug)
		: SNRThresholdDeciderOld(phy, threshold, sensitivity, myIndex, debug)
	{

		// commented out, because TestBaseLayer no longer subclasses TestModule
		//init("decider" + toString(this->myIndex));
		assert(phy);

		// check if member 'myIndex' has been set properly
		assertEqual(log("Member 'myIndex' has been initialized properly by passed value."),
				this->myIndex, myIndex);

		// check if everything else has been set properly
		checkInitMembers(phy, threshold, sensitivity);
	}
};


#endif /* TESTSNRTHRESHOLDDECIDER_H_ */
