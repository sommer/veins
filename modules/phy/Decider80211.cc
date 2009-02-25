/*
 * Decider80211.cc
 *
 *  Created on: 11.02.2009
 *      Author: karl wessel
 */

#include "Decider80211.h"
#include <DeciderResult80211.h>
#include <Mac80211Pkt_m.h>


// TODO: for now we check a larger mapping within an interval
/**
 * @brief Checks a mapping against a specific threshold (element-wise).
 *
 * @return	false	, if there exists an entry smaller than threshold
 * 			true	, otherwise
 *
 *
 */
DeciderResult* Decider80211::checkIfSignalOk(Mapping* snrMap, AirFrame* frame)
{
	assert(snrMap);

	Signal& s = frame->getSignal();
	simtime_t start = s.getSignalStart();
	simtime_t end = start + s.getSignalLength();

	start = start + RED_PHY_HEADER_DURATION; //its ok if the phy header is received only
											 //partly - TODO: maybe solve this nicer
	double snirMin = MappingUtils::findMin(*snrMap, Argument(start), Argument(end));

	EV << " snrMin: " << snirMin << endl;

	ConstMappingIterator* bitrateIt = s.getBitrate()->createConstIterator();
	bitrateIt->next(); //iterate to payload bitrate indicator
	double payloadBitrate = bitrateIt->getValue();
	delete bitrateIt;

	DeciderResult80211* result = 0;

	//TOPORT: check why mobility-fws Decider checks for rxDuration
	if (snirMin > snrThreshold) {
		if(packetOk(snirMin, frame->getBitLength() - (int)PHY_HEADER_LENGTH, payloadBitrate)) {
			result = new DeciderResult80211(true, payloadBitrate, snirMin);
		} else {
			EV << "Packet has BIT ERRORS! It is lost!\n";
			result = new DeciderResult80211(false, payloadBitrate, snirMin);
		}
	} else {
		EV << "Packet has ERRORS! It is lost!\n";
		result = new DeciderResult80211(false, payloadBitrate, snirMin);
	}

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
        berMPDU = 2.0 * (1.0 - 1.0 / sqrt(pow(2.0, 4))) * erfc(sqrt(2.0*snirMin * BANDWIDTH / bitrate));
    }
    else {                       // CCK, modelled with 256-QAM
        berMPDU = 2.0 * (1.0 - 1.0 / sqrt(pow(2.0, 8))) * erfc(sqrt(2.0*snirMin * BANDWIDTH / bitrate));
    }

    //probability of no bit error in the PLCP header
    double headerNoError = pow(1.0 - berHeader, HEADER_WITHOUT_PREAMBLE);

    //probability of no bit error in the MPDU
    double MpduNoError = pow(1.0 - berMPDU, lengthMPDU);
    EV << "berHeader: " << berHeader << " berMPDU: " << berMPDU << endl;
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

/**
 * @brief Processes a received AirFrame.
 *
 * The SNR-mapping for the Signal is created and checked against the Deciders
 * SNR-threshold. Depending on that the received AirFrame is either sent up
 * to the MAC-Layer or dropped.
 *
 * @return	usually return a value for: 'do not pass it again'
 */
simtime_t Decider80211::processSignalEnd(AirFrame* frame)
{
	// here the Signal is finally processed

	// first collect all necessary information
	Mapping* snrMap = calculateSnrMapping(frame);
	assert(snrMap);

	DeciderResult* result = checkIfSignalOk(snrMap, frame);

	// check if the snrMapping is above the Decider's specific threshold,
	// i.e. the Decider has received it correctly
	if (result->isSignalCorrect())
	{
		EV << "packet was received correctly, it is now handed to upper layer...\n";
		// go on with processing this AirFrame, send it to the Mac-Layer
		phy->sendUp(frame, result);
	} else
	{
		EV << "packet was not received correctly, sending it as control message to upper layer\n";
		Mac80211Pkt* mac = static_cast<Mac80211Pkt*>(frame->decapsulate());
		mac->setName("ERROR");
		mac->setKind(BITERROR);
		phy->sendControlMsg(mac);
		delete result;
	}

	delete snrMap;
	snrMap = 0;


	// we have processed this AirFrame and we prepare to receive the next one
	currentSignal.first = 0;

	//channel is idle now
	setChannelIdleStatus(true);

	return notAgain;
}
