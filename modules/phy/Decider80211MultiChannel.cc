/*
 * Decider80211MultiChannel.cpp
 *
 *  Created on: Mar 22, 2011
 *      Author: karl
 */

#include "Decider80211MultiChannel.h"
#include "DeciderResult80211.h"

Decider80211MultiChannel::Decider80211MultiChannel(DeciderToPhyInterface* phy,
					double threshold,
					double sensitivity,
					double decodingCurrentDelta,
					int currentChannel,
					int myIndex,
					bool debug):
	Decider80211Battery(phy, threshold, sensitivity,
						0.0, decodingCurrentDelta,
						myIndex, debug),
	currentChannel(currentChannel)
{
	assert(1 <= currentChannel && currentChannel <= 14);
	centerFrequency = CENTER_FREQUENCIES[currentChannel];
}

Decider80211MultiChannel::~Decider80211MultiChannel() {}

void Decider80211MultiChannel::getChannelInfo(simtime_t start, simtime_t end, AirFrameVector& out)
{
	Decider80211Battery::getChannelInfo(start, end, out);

	for(AirFrameVector::iterator it = out.begin();
		it != out.end(); ++it)
	{
		AirFrame* af = *it;
		if(af->getChannel() != currentChannel) {
			it = out.erase(it);
			--it;
		}
	}
}

simtime_t Decider80211MultiChannel::processNewSignal(AirFrame* frame) {
	if(frame->getChannel() != currentChannel)
		return notAgain;

	return Decider80211Battery::processNewSignal(frame);
}

DeciderResult* Decider80211MultiChannel::checkIfSignalOk(AirFrame* frame) {
	DeciderResult* result = 0;

	if(frame->getChannel() != currentChannel) {
		ConstMappingIterator* bitrateIt
				= frame->getSignal().getBitrate()->createConstIterator();
		bitrateIt->next(); //iterate to payload bitrate indicator
		double payloadBitrate = bitrateIt->getValue();
		delete bitrateIt;
		EV << "Channel changed during reception. packet is lost!\n";
		result = new DeciderResult80211(false, payloadBitrate, 0);
	} else {
		result = Decider80211Battery::checkIfSignalOk(frame);
	}

	return result;
}

void Decider80211MultiChannel::channelChanged(int newChannel) {
	assert(1 <= currentChannel && currentChannel <= 14);
	currentChannel = newChannel;
	centerFrequency = CENTER_FREQUENCIES[currentChannel];
	channelStateChanged();
}
