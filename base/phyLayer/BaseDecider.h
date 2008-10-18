#ifndef BASEDECIDER_H_
#define BASEDECIDER_H_

#include "Decider.h"


class BaseDecider : public Decider
{
protected:

	// TODO: think of how to test the protected helper functions
	// (calculateSNRMapping(), etc.)

	typedef DeciderToPhyInterface::AirFrameVector AirFrameVector;

	// threshold value for checking a SNR-map (SNR-threshold)
	double snrThreshold;

	// sensitivity value for receiving an AirFrame
	double sensitivity;

	// pointer to the currently received AirFrame
	AirFrame* currentAirFrame;

	// pointer to the currently running ChannelSenseRequest and its start-time
	std::pair<ChannelSenseRequest*, simtime_t> currentChannelSenseRequest;

	// simtime that tells the Phy-Layer not to pass an AirFrame again
	const simtime_t notAgain;

	// index for this Decider-instance given by Phy-Layer (mostly Host-index)
	// default-value for myIndex is -1, i.e. no parameter passed to constructor-call
	int myIndex;

	// toggles display of debugging messages
	bool debug;

	/**
	 * @brief Calculates a SNR-Mapping for a Signal.
	 *
	 * Therefore a Noise-Strength-Mapping is calculated for the time-interval
	 * of the Signal and the Signal-Strength-Mapping is divided by the
	 * Noise-Strength-Mapping.
	 *
	 * Note: 'divided' means here the special element-wise operation on mappings.
	 *
	 */
	virtual Mapping* calculateSnrMapping(AirFrame* frame);

	/**
	 * @brief Calculates a RSSI-Mapping (or Noise-Strength-Mapping) for a Signal.
	 *
	 * This method can be used to calculate a RSSI-Mapping in case the parameter
	 * exclude is omitted OR to calculate a Noise-Strength-Mapping in case the
	 * AirFrame of the received Signal is passed as parameter exclude.
	 */
	virtual Mapping* calculateRSSIMapping(	simtime_t start,
											simtime_t end,
											AirFrame* exclude);

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
	 * @brief Handles a Signal that is new to the Decider
	 *
	 * @return	Time-point when the AirFrame shall be handed over again.
	 */
	virtual simtime_t handleNewSignal(AirFrame* frame);

	/**
	 * @brief Handles (processes) a Signal that has been listened to and that is now over.
	 *
	 * @return	usually return a value for: 'do not pass it again'
	 *
	 */
	virtual simtime_t handleSignalOver(AirFrame* frame);


	// --- Utility methods ---
	virtual bool currentlyIdle()
	{
		return (currentAirFrame == 0);
	}

	void log(std::string msg)
	{
		if ( myIndex == -1 )
		{
			ev << "[Host (not set)] - PhyLayer(Decider): " << msg << endl;
		}
		else
		{
			ev << "[Host " << myIndex << "] - PhyLayer(Decider): " << msg << endl;
		}
	}

	virtual void resetChannelSenseRequest()
	{
		currentChannelSenseRequest.first = 0;
		currentChannelSenseRequest.second = -1;
	}


public:

	/**
	 * @brief Initializes the Decider with a pointer to its PhyLayer and
	 * specific values for threshold and sensitivity
	 */
	BaseDecider(DeciderToPhyInterface* phy,
				double threshold,
				double sensitivity,
				int myIndex = -1,
				bool debug = false)
		: Decider(phy), snrThreshold(threshold), sensitivity(sensitivity), notAgain(-1),
		myIndex(myIndex), debug(debug)
	{
		currentAirFrame = 0;
		currentChannelSenseRequest = std::pair<ChannelSenseRequest*, simtime_t>(0, -1);
	}

	virtual ~BaseDecider() {};

	/**
	 * @brief This function processes a AirFrame given by the PhyLayer and
	 * returns the time point when BaseDecider wants to be given the AirFrame again.
	 *
	 * BaseDecider decides whether it receives an AirFrame when it is passed for
	 * the first time and, if so, schedules it to the end of the AirFrame for the
	 * second hand-over.
	 *
	 * These cases is distinguished between:
	 *
	 * AirFrame F is handed over.
	 * 		1.) currentAirFrame == 0
	 * 			(we're not already receiving an AirFrame)
	 * 			a)  recv.-power of F is too low / --
	 * 				(do nothing)
	 * 			b)	recv.-power of F is sufficient / currentAirFrame = F; schedule F to its end-time
	 * 				(we're starting to receive F)
	 * 		2.) currentAirFrame == F / process F; currentAirFrame = 0 when done
	 * 			(we're already receiving F and are handed over F again --> process F)
	 * 		3.) currentAirFrame == G  (G != F) / --
	 * 			(we're currently receiving another AirFrame, or something else happened, do nothing)
	 *
	 *
	 *
	 */
	virtual simtime_t processSignal(AirFrame* frame);


	/**
	 * @brief A function that returns information about the channel state
	 *
	 * It is an alternative for the MACLayer in order to obtain information
	 * immediately (in contrast to sending a ChannelSenseRequest,
	 * i.e. sending a cMessage over the OMNeT-control-channel)
	 */
	virtual ChannelState getChannelState();

	/**
	 * @brief This function is called by the PhyLayer to hand over a
	 * ChannelSenseRequest.
	 *
	 * The MACLayer is able to send a ChannelSenseRequest to the PhyLayer
	 * that calls this function with it and is returned a time point when to
	 * re-call this function with the specific ChannelSenseRequest.
	 *
	 * The BaseDecider puts the result (ChannelState) to the ChannelSenseRequest
	 * and "answers" by calling the "sendControlMsg"-function on the
	 * DeciderToPhyInterface, i.e. telling the PhyLayer to send it back.
	 */
	virtual simtime_t handleChannelSenseRequest(ChannelSenseRequest* request);



};

#endif /* BASEDECIDER_H_ */
