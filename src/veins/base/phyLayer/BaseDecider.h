/*
 * BaseDecider.h
 *
 *  Created on: 24.02.2009
 *      Author: karl
 */

#ifndef BASEDECIDER_H_
#define BASEDECIDER_H_

#include "veins/base/utils/MiXiMDefs.h"
#include "veins/base/phyLayer/Decider.h"

class Mapping;

using Veins::AirFrame;

#define deciderEV EV << "[Host " << myIndex << "] - PhyLayer(Decider): "

/**
 * @brief Provides some base functionality for most common deciders.
 *
 * Forwards the AirFrame from "processSignal" to "processNewSignal",
 * "processSignalHeader" or "processSignalEnd" depending on the
 * state for that AirFrame returned by "getSignalState".
 *
 * Provides answering of ChannelSenseRequests (instantaneous and over time).
 *
 * Subclasses should define when they consider the channel as idle by
 * calling "setChannelIdleStatus" because BaseDecider uses that to
 * answer ChannelSenseRequests.
 *
 * If a subclassing Decider only tries to receive one signal at a time
 * it can use BaseDeciders "currentSignal" member which is a pair of
 * the signal to receive and the state for that signal. The state
 * is then used by BaseDeciders "getSignalState" to decide to which
 * "process***" method to forward the signal.
 * If a subclassing Decider needs states for more than one Signal it
 * has to store these states by itself and should probably override
 * the "getSignalState" method.
 *
 * @ingroup decider
 * @ingroup baseModules
 */
class MIXIM_API BaseDecider: public Decider {
public:
	/**
	 * @brief The kinds of ControlMessages this Decider sends.
	 *
	 * Sub-classing decider should begin their own kind enumeration
	 * at the value of "LAST_BASE_DECIDER_CONTROL_KIND".
	 */
	enum BaseDeciderControlKinds {
		/** @brief The phy has recognized a bit error in the packet.*/
		PACKET_DROPPED = 22100,
		/** @brief Sub-classing decider should begin their own kinds at this
		 * value.*/
		LAST_BASE_DECIDER_CONTROL_KIND
	};

protected:

	/** @brief The current state of processing for a signal*/
	enum SignalState {
		/** @brief Signal is received the first time. */
		NEW,
		/** @brief Waiting for the header of the signal. */
		EXPECT_HEADER,
		/** @brief Waiting for the end of the signal. */
		EXPECT_END,
	};

	/** @brief sensitivity value for receiving an AirFrame */
	double sensitivity;

	/** @brief Pair of a AirFrame and the state it is in. */
	typedef std::pair<AirFrame*, int> ReceivedSignal;

	/** @brief pointer to the currently received AirFrame */
	ReceivedSignal currentSignal;

	/** @brief Stores the idle state of the channel.*/
	bool isChannelIdle;

	/** @brief Data about an currently ongoing ChannelSenseRequest. */
	typedef struct{
		ChannelSenseRequest* first;
		simtime_t second;
		simtime_t canAnswerAt;

		ChannelSenseRequest* getRequest() const { return first; }
		void setRequest(ChannelSenseRequest* request) { first = request; }
		simtime_t_cref getSenseStart() const { return second; }
		void setSenseStart(simtime_t_cref start) { second = start; }
		simtime_t_cref getAnswerTime() const { return canAnswerAt; }
		void setAnswerTime(simtime_t_cref answerAt) { canAnswerAt = answerAt; }
	} CSRInfo;

	/** @brief pointer to the currently running ChannelSenseRequest and its
	 * start-time */
	CSRInfo currentChannelSenseRequest;

	/** @brief index for this Decider-instance given by Phy-Layer (mostly
	 * Host-index) */
	int myIndex;

	/** @brief toggles display of debugging messages */
	bool debug;

public:
	/**
	 * @brief Initializes the decider with the passed values.
	 *
	 * Needs a pointer to its physical layer, the sensitivity, the index of the
	 * host and the debug flag.
	 */
	BaseDecider(DeciderToPhyInterface* phy, double sensitivity,
				int myIndex, bool debug):
		Decider(phy),
		sensitivity(sensitivity),
		isChannelIdle(true),
		myIndex(myIndex),
		debug(debug)
	{
		currentSignal.first = 0;
		currentSignal.second = NEW;
		currentChannelSenseRequest.first = 0;
		currentChannelSenseRequest.second = -1;
		currentChannelSenseRequest.canAnswerAt = -1;
	}

	virtual ~BaseDecider() {}

public:
	/**
	 * @brief Processes an AirFrame given by the PhyLayer
	 *
	 * Returns the time point when the decider wants to be given the AirFrame
	 * again.
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
	 * The Decider puts the result (ChannelState) to the ChannelSenseRequest
	 * and "answers" by calling the "sendControlMsg"-function on the
	 * DeciderToPhyInterface, i.e. telling the PhyLayer to send it back.
	 */
	virtual simtime_t handleChannelSenseRequest(ChannelSenseRequest* request);



protected:
	/**
	 * @brief Processes a new Signal. Returns the time it wants to
	 * handle the signal again.
	 *
	 * Default implementation checks if the signals receiving power
	 * is above the sensitivity of the radio and we are not already trying
	 * to receive another AirFrame. If thats the case it waits for the end
	 * of the signal.
	 */
	virtual simtime_t processNewSignal(AirFrame* frame);

	/**
	 * @brief Processes the end of the header of a received Signal.
	 *
	 * Returns the time it wants to handle the signal again.
	 *
	 * Default implementation does not handle signal headers.
	 */
	virtual simtime_t processSignalHeader(AirFrame* frame) {
		throw cRuntimeError("BaseDecider does not handle Signal headers!");
		return notAgain;
	}

	/**
	 * @brief Processes the end of a received Signal.
	 *
	 * Returns the time it wants to handle the signal again
	 * (most probably notAgain).
	 *
	 * Default implementation just decides every signal as correct and passes it
	 * to the upper layer.
	 */
	virtual simtime_t processSignalEnd(AirFrame* frame);

	/**
	 * @brief Processes any Signal for which no state could be found.
	 * (is an error case).
	 */
	virtual simtime_t processUnknownSignal(AirFrame* frame);

	/**
	 * @brief Returns the SignalState for the passed AirFrame.
	 *
	 * The default implementation checks if the passed AirFrame
	 * is the "currentSignal" and returns its state or if not
	 * "NEW".
	 */
	virtual int getSignalState(AirFrame* frame);

	/**
	 * @brief Handles a new incoming ChannelSenseRequest and returns the next
	 * (or latest) time to handle the request again.
	 */
	virtual simtime_t handleNewSenseRequest(ChannelSenseRequest* request);

	/**
	 * @brief Handles the timeout or end of a ChannelSenseRequest by calculating
	 * the ChannelState and returning the request to the mac layer.
	 *
	 * If this handler is reached the decider has to be able to answer the
	 * request. Either because the timeout is reached or because the
	 * channel state changed accordingly.
	 */
	virtual void handleSenseRequestEnd(CSRInfo& requestInfo);

	/**
	 * @brief Changes the "isIdle"-status to the passed value.
	 *
	 * This method further checks if there are any ChannelSenseRequests
	 * which can be answered because of the idle state changed.
	 */
	virtual void setChannelIdleStatus(bool isIdle);

	/**
	 * @brief Returns point in time when the ChannelSenseRequest of the passed
	 * CSRInfo can be answered (e.g. because channel state changed or timeout
	 * is reached).
	 */
	virtual simtime_t canAnswerCSR(const CSRInfo& requestInfo);

	/**
	 * @brief Calculates the RSSI value for the passed interval.
	 *
	 * This method is called by BaseDecider when it answers a
	 * ChannelSenseRequest or calculates the channel state. Can be overridden
	 * by sub classing Deciders.
	 *
	 * Default implementation returns the maximum RSSI value inside the
	 * passed interval.
	 */
	virtual double calcChannelSenseRSSI(simtime_t_cref start, simtime_t_cref end);

	/**
	 * @brief Answers the ChannelSenseRequest (CSR) from the passed CSRInfo.
	 *
	 * Calculates the rssi value and the channel idle state and sends the CSR
	 * together with the result back to the mac layer.
	 */
	virtual void answerCSR(CSRInfo& requestInfo);

	/**
	 * @brief Checks if the changed channel state enables us to answer
	 * any ongoing ChannelSenseRequests.
	 *
	 * This method is ment to update only an already ongoing
	 * ChannelSenseRequests it can't handle a new one.
	 */
	virtual void channelStateChanged();

	/**
	 * @brief Collects the AirFrame on the channel during the passed interval.
	 *
	 * Forwards to DeciderToPhyInterfaces "getChannelInfo" method.
	 * Subclassing deciders can override this method to filter the returned
	 * AirFrames for their own criteria, for example by removing AirFrames on
	 * another not interferring channel.
	 *
	 * @param start The start of the interval to collect AirFrames from.
	 * @param end The end of the interval to collect AirFrames from.
	 * @param out The output vector in which to put the AirFrames.
	 */
	virtual void getChannelInfo(simtime_t_cref start, simtime_t_cref end,
								AirFrameVector& out);

	//------Utility methods------------

	/**
	 * @brief Calculates a SNR-Mapping for a Signal.
	 *
	 * A Noise-Strength-Mapping is calculated (by using the
	 * "calculateRSSIMapping()"-method) for the time-interval
	 * of the Signal and the Signal-Strength-Mapping is divided by the
	 * Noise-Strength-Mapping.
	 *
	 * Note: 'divided' means here the special element-wise operation on
	 * mappings.
	 */
	virtual Mapping* calculateSnrMapping(AirFrame* frame);

	/**
	 * @brief Calculates a RSSI-Mapping (or Noise-Strength-Mapping) for a
	 * Signal.
	 *
	 * This method can be used to calculate a RSSI-Mapping in case the parameter
	 * exclude is omitted OR to calculate a Noise-Strength-Mapping in case the
	 * AirFrame of the received Signal is passed as parameter exclude.
	 */
	virtual Mapping* calculateRSSIMapping(	simtime_t_cref start,
											simtime_t_cref end,
											AirFrame*      exclude = NULL);
};

#endif /* BASEDECIDER_H_ */
