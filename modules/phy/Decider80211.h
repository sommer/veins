/*
 * Decider80211.h
 *
 *  Created on: 11.02.2009
 *      Author: karl wessel
 */

#ifndef DECIDER80211_H_
#define DECIDER80211_H_

#include "Decider.h"
#include "Consts80211.h"

/**
 * @brief Decider for the 802.11 modules
 *
 * Depending on the minimum of the snr included in the PhySDU this
 * module computes a bit error probability. The header (1 Mbit/s) is
 * always modulated with DBQPSK. The PDU is normally modulated either
 * with DBPSK (1 and 2 Mbit/s) or CCK (5.5 and 11 Mbit/s). CCK is not
 * easy to model, therefore it is modeled as DQPSK with a 16-QAM for
 * 5.5 Mbit/s and a 256-QAM for 11 Mbit/s.
 *
 *
 * @ingroup decider
 * @author Marc L�bbers, David Raguin, Karl Wessel(port for MiXiM)
 */
class Decider80211: public Decider {
protected:

	// defines what an AirFrameVector shall be here
	typedef DeciderToPhyInterface::AirFrameVector AirFrameVector;

	// threshold value for checking a SNR-map (SNR-threshold)
	double snrThreshold;

	// sensitivity value for receiving an AirFrame
	double sensitivity;

	// pointer to the currently received AirFrame
	AirFrame* currentAirFrame;

	/** @brief Pair of a ChannelSenseRequest and the simtime it started. */
	typedef std::pair<ChannelSenseRequest*, simtime_t> CSRInfo;

	// pointer to the currently running ChannelSenseRequest and its start-time
	CSRInfo currentChannelSenseRequest;

	// simtime that tells the Phy-Layer not to pass an AirFrame again
	const simtime_t notAgain;

	// index for this Decider-instance given by Phy-Layer (mostly Host-index)
	// default-value for myIndex is -1, i.e. no parameter passed to constructor-call
	int myIndex;

	// toggles display of debugging messages
	bool debug;


protected:

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
	virtual DeciderResult* checkIfSignalOk(Mapping* snrMap, AirFrame* frame);

	/**
	 * @brief Handles a newly arrived AirFrame, i.e. check whether the receiving power
	 * of the Signal is high enough and then focus on that AirFrame (receive it) or not.
	 *
	 * @return	Time point when the AirFrame shall be handed over again.
	 *
	 */
	virtual simtime_t handleNewSignal(AirFrame* frame);

	/**
	 * @brief Processes a received AirFrame.
	 *
	 * The SNR-mapping for the Signal is created and checked against the Deciders
	 * SNR-threshold. Depending on that the received AirFrame is either sent up
	 * to the MAC-Layer or dropped.
	 *
	 * @return	usually return a value for: 'do not pass it again'
	 */
	virtual simtime_t handleSignalOver(AirFrame* frame);

	/**
	 * @brief handles a new incoming ChannelSenseRequest and returns the next
	 * (or latest) time to handle the request again.
	 */
	virtual simtime_t handleNewSenseRequest(ChannelSenseRequest* request);

	/**
	 * @brief Handles the timeout of a ChannelSenseRequest by calculating the
	 * ChannelState and returning the request to the mac layer.
	 */
	virtual void handleSenseRequestTimeout(CSRInfo& requestInfo);

	/**
	 * @brief Returns true if the ChannelSenseRequest of the passed CSRInfo can be answered
	 * (e.g. because channel state changed or timeout is reached).
	 */
	virtual bool canAnswerCSR(const CSRInfo& requestInfo);

	/**
	 * @brief Answers the ChannelSenseRequest (CSR) from the passed CSRInfo by calculating
	 * the rssi value and the channel idle state and sending the CSR together with the result
	 * back to the mac layer.
	 */
	virtual void answerCSR(const CSRInfo& requestInfo);

	/** @brief computes if packet is ok or has errors*/
	bool packetOk(double snirMin, int lengthMPDU, double bitrate);

	// --- Utility methods ---
	/**
	 * @brief Defines when this specific Decider considers the channel to be idle.
	 *
	 */
	virtual bool currentlyIdle()
	{
		return (currentAirFrame == 0);
	}

	/**
	 * @brief Convenience method for logging output-messages.
	 *
	 */
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

	/**
	 * @brief Resets the currently handled ChannelSenseRequest.
	 *
	 */
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
	Decider80211(DeciderToPhyInterface* phy,
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

	virtual ~Decider80211() {};

	/**
	 * @brief This function processes a AirFrame given by the PhyLayer and
	 * returns the time point when SNRThresholdDecider wants to be given the AirFrame again.
	 *
	 * SNRThresholdDecider decides whether it receives an AirFrame when it is passed for
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
	 * The SNRThresholdDecider puts the result (ChannelState) to the ChannelSenseRequest
	 * and "answers" by calling the "sendControlMsg"-function on the
	 * DeciderToPhyInterface, i.e. telling the PhyLayer to send it back.
	 */
	virtual simtime_t handleChannelSenseRequest(ChannelSenseRequest* request);
};

#endif /* DECIDER80211_H_ */
