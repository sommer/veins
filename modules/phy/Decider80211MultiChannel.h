/*
 * Decider80211MultiChannel.h
 *
 *  Created on: Mar 22, 2011
 *      Author: karl
 */

#ifndef DECIDER80211MULTICHANNEL_H_
#define DECIDER80211MULTICHANNEL_H_

#include "AirFrameMultiChannel_m.h"

/*
 *
 */
#include "Decider80211Battery.h"

class Decider80211MultiChannel: public Decider80211Battery
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
	virtual void getChannelInfo(simtime_t start, simtime_t end, AirFrameVector& out)
	{
		Decider80211Battery::getChannelInfo(start, end, out);

		for(AirFrameVector::iterator it = out.begin();
			it != out.end(); ++it)
		{
			AirFrame* af = *it;
			assert(af);
			assert(dynamic_cast<AirFrameMultiChannel*>(af));
			AirFrameMultiChannel* afm = static_cast<AirFrameMultiChannel*>(af);
			if(afm->getChannel() != currentChannel) {
				it = out.erase(it);
				--it;
			}
		}
	}

	virtual simtime_t processNewSignal(AirFrame* frame) {
		assert(dynamic_cast<AirFrameMultiChannel*>(frame));
		AirFrameMultiChannel* af = static_cast<AirFrameMultiChannel*>(frame);
		if(af->getChannel() != currentChannel)
			return notAgain;

		return Decider80211Battery::processNewSignal(frame);
	}

	/**
	 * @brief Processes a received AirFrame.
	 *
	 * The SNR-mapping for the Signal is created and checked against the Deciders
	 * SNR-threshold. Depending on that the received AirFrame is either sent up
	 * to the MAC-Layer or dropped.
	 *
	 * @return	usually return a value for: 'do not pass it again'
	 */
	virtual simtime_t processSignalEnd(AirFrame* frame) {
		assert(dynamic_cast<AirFrameMultiChannel*>(frame));
		AirFrameMultiChannel* af = static_cast<AirFrameMultiChannel*>(frame);
		if(af->getChannel() != currentChannel) {
			//TODO: hand up broken packet to upper layer
			// we have processed this AirFrame and we prepare to receive the next one
			currentSignal.first = 0;

			//channel is idle now
			setChannelIdleStatus(true);
			return notAgain;
		}

		return Decider80211Battery::processSignalEnd(frame);
	}
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
						bool debug = false):
		Decider80211Battery(phy, threshold, sensitivity,
							0.0, decodingCurrentDelta,
							myIndex, debug),
		currentChannel(currentChannel)
	{
		assert(1 <= currentChannel && currentChannel <= 14);
		centerFrequency = CENTER_FREQUENCIES[currentChannel];
	}
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
	virtual void channelChanged(int newChannel) {
		assert(1 <= currentChannel && currentChannel <= 14);
		currentChannel = newChannel;
		centerFrequency = CENTER_FREQUENCIES[currentChannel];
		channelStateChanged();
	}


};

#endif /* DECIDER80211MULTICHANNEL_H_ */
