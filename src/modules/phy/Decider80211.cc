/*
 * Decider80211.cc
 *
 *  Created on: 11.02.2009
 *      Author: karl wessel
 */

#include "Decider80211.h"

#include <cassert>

#include "DeciderResult80211.h"
#include "Mac80211Pkt_m.h"
#include "Consts80211.h"
#include "Mapping.h"
#include "AirFrame_m.h"
#include "FWMath.h"

Decider80211::Decider80211(	DeciderToPhyInterface* phy,
							double threshold,
							double sensitivity,
							int channel,
							int myIndex,
							bool debug):
	BaseDecider(phy, sensitivity, myIndex, debug),
	snrThreshold(threshold)
{
	assert(1 <= channel);
	assert(channel  <= 14);
	centerFrequency = CENTER_FREQUENCIES[channel];
}

simtime_t Decider80211::processNewSignal(AirFrame* frame) {
	if(currentSignal.first != 0) {
		deciderEV << "Already receiving another AirFrame!" << endl;
		return notAgain;
	}

	// get the receiving power of the Signal at start-time and center frequency
	Signal& signal = frame->getSignal();
	Argument start(DimensionSet::timeFreqDomain);
	start.setTime(signal.getReceptionStart());
	start.setArgValue(Dimension::frequency, centerFrequency);

	double recvPower = signal.getReceivingPower()->getValue(start);

	// check whether signal is strong enough to receive
	if ( recvPower < sensitivity )
	{
		deciderEV << "Signal is to weak (" << recvPower << " < " << sensitivity
				<< ") -> do not receive." << endl;
		// Signal too weak, we can't receive it, tell PhyLayer that we don't want it again
		return notAgain;
	}

	// Signal is strong enough, receive this Signal and schedule it
	deciderEV << "Signal is strong enough (" << recvPower << " > " << sensitivity
			<< ") -> Trying to receive AirFrame." << endl;

	currentSignal.first = frame;
	currentSignal.second = EXPECT_END;

	//channel turned busy
	setChannelIdleStatus(false);

	return ( signal.getReceptionEnd());
}

double Decider80211::calcChannelSenseRSSI(simtime_t_cref start, simtime_t_cref end) {
	Mapping* rssiMap = calculateRSSIMapping(start, end);

	Argument min(DimensionSet::timeFreqDomain);
	min.setTime(start);
	min.setArgValue(Dimension::frequency, centerFrequency - 11e6);
	Argument max(DimensionSet::timeFreqDomain);
	max.setTime(end);
	max.setArgValue(Dimension::frequency, centerFrequency + 11e6);

	Mapping::argument_value_t rssi = MappingUtils::findMax(*rssiMap, min, max, Argument::MappedZero /* the value if no maximum will be found */);

	delete rssiMap;
	return rssi;
}


DeciderResult* Decider80211::checkIfSignalOk(AirFrame* frame)
{
	// check if the snrMapping is above the Decider's specific threshold,
	// i.e. the Decider has received it correctly

	// first collect all necessary information
	Mapping* snrMap = calculateSnrMapping(frame);
	assert(snrMap);

	Signal& s = frame->getSignal();
	simtime_t start = s.getReceptionStart();
	simtime_t end = s.getReceptionEnd();

	start = start + RED_PHY_HEADER_DURATION; //its ok if the phy header is received only
											 //partly - TODO: maybe solve this nicer
	Argument min(DimensionSet::timeFreqDomain);
	min.setTime(start);
	min.setArgValue(Dimension::frequency, centerFrequency - 11e6);
	Argument max(DimensionSet::timeFreqDomain);
	max.setTime(end);
	max.setArgValue(Dimension::frequency, centerFrequency + 11e6);

	Mapping::argument_value_t snirMin = MappingUtils::findMin(*snrMap, min, max, Argument::MappedZero /* the value if no minimum will be found */);

	deciderEV << " snrMin: " << snirMin << endl;

	ConstMappingIterator* bitrateIt = s.getBitrate()->createConstIterator();
	bitrateIt->next(); //iterate to payload bitrate indicator
	double payloadBitrate = bitrateIt->getValue();
	delete bitrateIt;

	DeciderResult80211* result = 0;

	if (snirMin > snrThreshold) {
		if(packetOk(snirMin, frame->getBitLength() - (int)PHY_HEADER_LENGTH, payloadBitrate)) {
			result = new DeciderResult80211(true, payloadBitrate, snirMin);
		} else {
			deciderEV << "Packet has BIT ERRORS! It is lost!\n";
			result = new DeciderResult80211(false, payloadBitrate, snirMin);
		}
	} else {
		deciderEV << "Packet has ERRORS! It is lost!\n";
		result = new DeciderResult80211(false, payloadBitrate, snirMin);
	}

	delete snrMap;
	snrMap = 0;

	return result;
}

bool Decider80211::packetOk(double snirMin, int lengthMPDU, double bitrate)
{
    double berHeader, berMPDU;

    berHeader = 0.5 * exp(-snirMin * BANDWIDTH / BITRATE_HEADER);
    //if PSK modulation
    if (bitrate == 1E+6 || bitrate == 2E+6) {
        berMPDU = 0.5 * exp(-snirMin * BANDWIDTH / bitrate);
    }
    //if CCK modulation (modeled with 16-QAM)
    else if (bitrate == 5.5E+6) {
        berMPDU = 2.0 * (1.0 - 1.0 / sqrt(pow(2.0, 4))) * FWMath::erfc(sqrt(2.0*snirMin * BANDWIDTH / bitrate));
    }
    else {                       // CCK, modelled with 256-QAM
        berMPDU = 2.0 * (1.0 - 1.0 / sqrt(pow(2.0, 8))) * FWMath::erfc(sqrt(2.0*snirMin * BANDWIDTH / bitrate));
    }

    //probability of no bit error in the PLCP header
    double headerNoError = pow(1.0 - berHeader, HEADER_WITHOUT_PREAMBLE);

    //probability of no bit error in the MPDU
    double MpduNoError = pow(1.0 - berMPDU, lengthMPDU);
    deciderEV << "berHeader: " << berHeader << " berMPDU: " << berMPDU << endl;
    double rand = dblrand();

    //if error in header
    if (rand > headerNoError)
        return (false);
    else
    {
        rand = dblrand();

        //if error in MPDU
        if (rand > MpduNoError)
            return (false);
        //if no error
        else
            return (true);
    }
}

simtime_t Decider80211::processSignalEnd(AirFrame* frame)
{
	// here the Signal is finally processed

	DeciderResult* result = checkIfSignalOk(frame);

	if (result->isSignalCorrect())
	{
		deciderEV << "packet was received correctly, it is now handed to upper layer...\n";
		// go on with processing this AirFrame, send it to the Mac-Layer
		phy->sendUp(frame, result);
	} else
	{
		deciderEV << "packet was not received correctly, sending it as control message to upper layer\n";
		Mac80211Pkt* mac = static_cast<Mac80211Pkt*>(frame->decapsulate());
		mac->setName("ERROR");
		mac->setKind(BITERROR);
		phy->sendControlMsgToMac(mac);
		delete result;
	}

	// we have processed this AirFrame and we prepare to receive the next one
	currentSignal.first = 0;

	//channel is idle now
	setChannelIdleStatus(true);

	return notAgain;
}
