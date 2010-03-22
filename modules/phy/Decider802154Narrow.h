/*
 * Decider80211.h
 *
 *  Created on: 11.02.2009
 *      Author: karl wessel
 */

#ifndef DECIDER802154NARROW_H_
#define DECIDER802154NARROW_H_

#include <BaseDecider.h>

/**
 * @brief Decider for the 802.15.4 Narrow band module
 *
 * @ingroup decider
 * @ingroup ieee802154
 * @author Jerome Rousselot, Amre El-Hoiydi, Marc Loebbers, Karl Wessel(port for MiXiM)
 */
class Decider802154Narrow: public BaseDecider {
protected:


	/** @brief Start Frame Delimiter length in bits. */
	int sfdLength;

	/** @brief Minimum bit error rate. If SNIR is high, computed ber could be
		higher than maximum radio performance. This value is an upper bound to
		the performance. */
	double BER_LOWER_BOUND;

	/** @brief modulation type */
	std::string modulation;

	/** @name Tracked statistic values.*/
	/*@{*/
	unsigned long nbFramesWithInterference;
	unsigned long nbFramesWithoutInterference;

	unsigned long nbFramesWithInterferenceDropped;
	unsigned long nbFramesWithoutInterferenceDropped;
	/*@}*/

protected:
	/** @brief Process a new signal the first time.*/
	virtual simtime_t processNewSignal(AirFrame* frame);

	/**
	 * @brief Process the end of a signal.
	 *
	 * Checks if signal was received correct and sends it
	 * up to the MAC layer.
	 */
	virtual simtime_t processSignalEnd(AirFrame* frame);

	double getBERFromSNR(double snr);

	bool syncOnSFD(AirFrame* frame);

	double evalBER(AirFrame* frame);

public:

	/**
	 * @brief Initializes the Decider with a pointer to its PhyLayer and
	 * specific values for threshold and sensitivity
	 */
	Decider802154Narrow(DeciderToPhyInterface* phy,
						int myIndex,
						bool debug,
						int sfdLength,
						double BER_LOWER_BOUND,
						const std::string& modulation):
		BaseDecider(phy, 0, myIndex, debug),
		sfdLength(sfdLength),
		BER_LOWER_BOUND(BER_LOWER_BOUND),
		modulation(modulation),
		nbFramesWithInterference(0),
		nbFramesWithoutInterference(0),
		nbFramesWithInterferenceDropped(0),
		nbFramesWithoutInterferenceDropped(0)
	{
		//TODO: publish initial rssi/channel state
		//TODO: trace noise level, snr and rssi to vectors
	}

	virtual ~Decider802154Narrow() {};

	/**
	 * @brief Method to be called by an OMNeT-module during its own finish(),
	 * to enable a decider to do some things.
	 */
	virtual void finish();
};

#endif /* DECIDER80211_H_ */
