/*
 * Decider80211MultiChannel.h
 *
 *  Created on: Mar 22, 2011
 *      Author: karl
 */

#ifndef DECIDER80211MULTICHANNEL_H_
#define DECIDER80211MULTICHANNEL_H_

#include "MiXiMDefs.h"
#include "Decider80211Battery.h"

/**
 * @brief Extends Decider80211 by multi channel support.
 *
 * Filters processing of AirFrames depending on the channel they were sent on
 * and the channel this deciders radio is currently set to. All AirFrames
 * on another channel than the currently used one are ignored totally, they are
 * neither received nor considered as interference.
 * If the channel changed during the reception of an AirFrame the AirFrame is
 * considered lost, independent from the size of the part which was lost due to
 * channel change.
 *
 * NOTE: This decider does not model interference between adjacent channels!
 *
 * @author Karl Wessel
 */
class MIXIM_API Decider80211MultiChannel: public Decider80211Battery
{
protected:
	int currentChannel;

protected:
	/**
	 * @brief Collects the AirFrame on the channel during the passed interval.
	 *
	 * Filters all AirFrames not on the same channel as this deciders radio.
	 *
	 * @param start The start of the interval to collect AirFrames from.
	 * @param end The end of the interval to collect AirFrames from.
	 * @param out The output vector in which to put the AirFrames.
	 */
	virtual void getChannelInfo(simtime_t_cref start, simtime_t_cref end,
								AirFrameVector& out);

	/**
	 * @brief Filters AirFrames on other channels than the current one.
	 *
	 * See processNewSignal of Decider80211 for details.
	 *
	 * @param frame The AirFrame to process.
	 * @return The time the AirFrame should be handled next by this decider.
	 */
	virtual simtime_t processNewSignal(AirFrame* frame);


	/**
	 * @brief Checks if the passed completed AirFrame was received correctly.
	 *
	 * Returns the result as a DeciderResult.
	 * If the channel changed during transmission the AirFrame is considered
	 * broken.
	 *
	 * @return	The result of the decider for the passed AirFrame.
	 */
	virtual DeciderResult* checkIfSignalOk(AirFrame* frame);

public:
	/**
	 * @brief Initializes this Decider with the passed values.
	 *
	 * @param phy Pointer to this deciders phy layer
	 * @param threshold The SNR threshold above which reception is correct
	 * @param sensitivity The strength (mW) at which a signal can be received
	 * @param centerFrequency The frequency used by the phy layer
	 * @param decodingCurrentDelta The additional amount of power it takes to
	 *        decode a signal
	 * @param myIndex The index of this deciders host (for debug output)
	 * @param debug Use debug output?
	 */
	Decider80211MultiChannel(DeciderToPhyInterface* phy,
						double threshold,
						double sensitivity,
						double decodingCurrentDelta,
						int currentChannel,
						int myIndex = -1,
						bool debug = false);

	virtual ~Decider80211MultiChannel();

	/**
	 * @brief Called by phy layer to indicate that the channel this radio
	 * currently listens to has changed.
	 *
	 * Sub-classing deciders which support multiple channels should override
	 * this method to handle the effects of channel changes on ongoing
	 * receptions.
	 *
	 * @param newChannel The new channel the radio has changed to.
	 */
	virtual void channelChanged(int newChannel);


};

#endif /* DECIDER80211MULTICHANNEL_H_ */
