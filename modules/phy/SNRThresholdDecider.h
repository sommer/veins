#ifndef SNRTHRESHOLDDECIDER_H_
#define SNRTHRESHOLDDECIDER_H_

#include <BaseDecider.h>

/**
 * @brief BaseDecider implementation which decides a signals
 * correctness by checking its SNR against a threshold.
 *
 * Decides the channel state (idle/busy) at hand of the current
 * received total power level (independent from signal or noise).
 * If its above the threshold defined by the "busyThreshold" parameter
 * it considers the channel busy.
 * The RSSI value returned by this Decider for a ChannelSenseRequest
 * over time is always the RSSI value at the end of the sense.
 *
 * @ingroup decider
 */
class SNRThresholdDecider : public BaseDecider
{
protected:
	/** @brief Threshold value for checking a SNR-map (SNR-threshold).*/
	double snrThreshold;

	/** @brief The threshold rssi level above which the channel is considered busy.*/
	double busyThreshold;

protected:

	/**
	 * @brief Checks a mapping against a specific threshold (element-wise).
	 *
	 * @return	true	, if every entry of the mapping is above threshold
	 * 			false	, otherwise
	 *
	 *
	 */
	virtual bool checkIfAboveThreshold(Mapping* map, simtime_t start, simtime_t end);

	/**
	 * @brief Processes a new Signal. Returns the time it wants to
	 * handle the signal again.
	 *
	 * Checks if the signals receiving power is above the sensitivity of
	 * the radio and we are not already trying to receive another AirFrame.
	 * If thats the case it waits for the end of the signal.
	 *
	 * Also checks if the new AirFrame changed the power level in the way
	 * that we can answer an ongoing channel sense request.
	 */
	virtual simtime_t processNewSignal(AirFrame* frame);

	/**
	 * @brief Processes a received AirFrame.
	 *
	 * The SNR-mapping for the Signal is created and checked against the Deciders
	 * SNR-threshold. Depending on that the received AirFrame is either sent up
	 * to the MAC-Layer or dropped.
	 *
	 * @return	usually return a value for: 'do not pass it again'
	 */
	virtual simtime_t processSignalEnd(AirFrame* frame);

	/**
	 * @brief Returns point in time when the ChannelSenseRequest of the passed CSRInfo can be answered
	 * (e.g. because channel state changed or timeout is reached).
	 */
	virtual simtime_t canAnswerCSR(const CSRInfo& requestInfo);

	/**
	 * @brief Answers the ChannelSenseRequest (CSR) from the passed CSRInfo.
	 *
	 * Calculates the rssi value and the channel idle state and sends the CSR
	 * together with the result back to the mac layer.
	 */
	virtual void answerCSR(CSRInfo& requestInfo);

	/**
	 * @brief Returns whether the passed rssi value indicates a idle channel.
	 * @param rssi the channels rssi value to evaluate
	 * @return true if the channel should be considered idle
	 */
	bool isIdleRSSI(double rssi) const {
		return rssi <= busyThreshold;
	}

public:

	/**
	 * @brief Initializes the Decider with a pointer to its PhyLayer and
	 * specific values for threshold and sensitivity
	 */
	SNRThresholdDecider(DeciderToPhyInterface* phy,
				double snrThreshold,
				double sensitivity,
				double busyThreshold,
				int myIndex = -1,
				bool debug = false):
		BaseDecider(phy, sensitivity, myIndex, debug),
		snrThreshold(snrThreshold),
		busyThreshold(busyThreshold)
	{}

	virtual ~SNRThresholdDecider() {};

	/**
	 * @brief A function that returns information about the channel state
	 *
	 * It is an alternative for the MACLayer in order to obtain information
	 * immediately (in contrast to sending a ChannelSenseRequest,
	 * i.e. sending a cMessage over the OMNeT-control-channel)
	 */
	virtual ChannelState getChannelState();
};

#endif /* BASEDECIDER_H_ */
