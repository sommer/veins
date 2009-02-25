#include "Decider.h"



/**
 * @brief A Function that returns a very basic result about the Signal.
 *
 */
bool DeciderResult::isSignalCorrect() {
	return isCorrect;
}

/**
 * @brief Initializes the Decider with a pointer to its PhyLayer
 */
Decider::Decider(DeciderToPhyInterface* phy):
	phy(phy), notAgain(-1) {}

/**
 * @brief This function processes a AirFrame given by the PhyLayer and
 * returns the time point when Decider wants to be given the AirFrame again
 *
 */
simtime_t Decider::processSignal(AirFrame* s) {

	//TODO: implement
	return -1;
}

/**
 * @brief A function that returns information about the channel state
 *
 * It is an alternative for the MACLayer in order to obtain information
 * immediately (in contrast to sending a ChannelSenseRequest,
 * i.e. sending a cMessage over the OMNeT-control-channel)
 *
 */
ChannelState Decider::getChannelState() {

	//TODO: implement
	return ChannelState();
}

/**
 * @brief This function is called by the PhyLayer to hand over a
 * ChannelSenseRequest.
 *
 * The MACLayer is able to send a ChannelSenseRequest to the PhyLayer
 * that calls this funtion with it and is returned a time point when to
 * re-call this function with the specific ChannelSenseRequest.
 *
 * The Decider puts the result (ChannelState) to the ChannelSenseRequest
 * and "answers" by calling the "sendControlMsg"-function on the
 * DeciderToPhyInterface, i.e. telling the PhyLayer to send it back.
 *
 */
simtime_t Decider::handleChannelSenseRequest(ChannelSenseRequest* request) {

	//TODO: implement
	return -1;
}
