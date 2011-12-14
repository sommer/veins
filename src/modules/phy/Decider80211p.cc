//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

/*
 * Based on Decider80211.cc from Karl Wessel
 * and modifications by Christopher Saloman
 */

#include <Decider80211p.h>
#include <DeciderResult80211.h>
#include <Mac80211Pkt_m.h>
#include <Signal_.h>
#include <AirFrame_m.h>

simtime_t Decider80211p::processNewSignal(AirFrame* frame) {
	if (currentSignal.first != 0) {
		DBG << "Already receiving another AirFrame!" << endl;
		return notAgain;
	}

	// get the receiving power of the Signal at start-time and center frequency
	Signal& signal = frame->getSignal();

	DBG << "Receiving Power" << endl;

	Argument start(DimensionSet::timeFreqDomain);
	start.setTime(signal.getReceptionStart());
	start.setArgValue(Dimension::frequency_static(), centerFrequency);

	double recvPower = signal.getReceivingPower()->getValue(start);

	// check whether signal is strong enough to receive
	DBG << "Decider currently on freq: " << centerFrequency << endl;
	if (recvPower < sensitivity) {
		DBG << "Signal is to weak (" << recvPower << " < " << sensitivity
		    << ") -> do not receive." << endl;
		// Signal too weak, we can't receive it, tell PhyLayer that we don't want it again
		return notAgain;
	}

	// Signal is strong enough, receive this Signal and schedule it
	DBG << "Signal is strong enough (" << recvPower << " > " << sensitivity
	    << ") -> Trying to receive AirFrame." << endl;

	currentSignal.first = frame;
	currentSignal.second = EXPECT_END;

	//measure communication density
	myBusyTime += signal.getDuration().dbl();

	//channel turned busy
	setChannelIdleStatus(false);

	return signal.getReceptionEnd();
}

double Decider80211p::calcChannelSenseRSSI(simtime_t_cref start, simtime_t_cref end) {

	Mapping* rssiMap = calculateRSSIMapping(start, end);

	Argument min(DimensionSet::timeFreqDomain);
	min.setTime(start);
	min.setArgValue(Dimension::frequency_static(), centerFrequency - 5e6);
	Argument max(DimensionSet::timeFreqDomain);
	max.setTime(end);
	max.setArgValue(Dimension::frequency_static(), centerFrequency + 5e6);

	double rssi = MappingUtils::findMax(*rssiMap, min, max);

	delete rssiMap;

	return rssi;
}


DeciderResult* Decider80211p::checkIfSignalOk(AirFrame* frame) {
	Mapping* snrMap = calculateSnrMapping(frame);
	assert(snrMap);

	Signal& s = frame->getSignal();
	simtime_t start = s.getReceptionStart();
	simtime_t end = s.getReceptionEnd();

	start = start + PHY_HDR_PREAMBLE_DURATION; //its ok if something in the training phase is broken

	Argument min(DimensionSet::timeFreqDomain);
	min.setTime(start);
	min.setArgValue(Dimension::frequency_static(), centerFrequency - 5e6);
	Argument max(DimensionSet::timeFreqDomain);
	max.setTime(end);
	max.setArgValue(Dimension::frequency_static(), centerFrequency + 5e6);

	double snirMin = MappingUtils::findMin(*snrMap, min, max);

	ConstMappingIterator* bitrateIt = s.getBitrate()->createConstIterator();
	bitrateIt->next(); //iterate to payload bitrate indicator
	double payloadBitrate = bitrateIt->getValue();
	delete bitrateIt;

	DeciderResult80211* result = 0;

	if (packetOk(snirMin, frame->getBitLength(), payloadBitrate)) {
		DBG << "Packet is fine! We can decode it" << std::endl;
		result = new DeciderResult80211(true, payloadBitrate, snirMin);
	}
	else {
		DBG << "Packet has bit Errors. Lost " << std::endl;
		result = new DeciderResult80211(false, payloadBitrate, snirMin);
	}

	delete snrMap;
	return result;
}

bool Decider80211p::packetOk(double snirMin, int lengthMPDU, double bitrate) {

	//the lengthMPDU includes the PHY_SIGNAL_LENGTH + PHY_PSDU_HEADER + Payload, while the first is sent with PHY_HEADER_BANDWIDTH

	double packetOk;

	if (bitrate == 18E+6) {
		//According to P. Fuxjaeger et al. "IEEE 802.11p Transmission Using GNURadio"
		double ber = std::min(0.5 , 1.5 * erfc(0.45 * sqrt(snirMin)));
		packetOk = pow(1 - ber, lengthMPDU - PHY_HDR_PLCPSIGNAL_LENGTH);
	}
	else if (bitrate == 6E+6) {
		//According to K. Sjoeberg et al. "Measuring and Using the RSSI of IEEE 802.11p"
		double ber = std::min(0.5 , 8 * erfc(0.75 *sqrt(snirMin)));
		packetOk = pow(1 - ber, lengthMPDU - PHY_HDR_PLCPSIGNAL_LENGTH);
	}
	else {
		opp_error("Currently this 11p-Model only provides accurate BER models for 6Mbit and 18Mbit. Please use one of these frequencies for now.");
	}

	//check if header is broken, BER model for PSK taken from MiXiM 2.2
	double berHeader = 0.5 * exp(-snirMin * 10E+6 / PHY_HDR_BANDWIDTH);
	double headerNoError = pow(1.0 - berHeader, PHY_HDR_PLCPSIGNAL_LENGTH);

	//probability of no bit error in the PLCP header

	double rand = dblrand();

	if (rand > headerNoError)
		return false;

	//probability of no bit error in the rest of the packet

	rand = dblrand();

	if (rand > packetOk)
		return false;
	else {
		return true;
	}
}

simtime_t Decider80211p::processSignalEnd(AirFrame* frame) {
	// here the Signal is finally processed

	// first collect all necessary information
	DeciderResult* result = checkIfSignalOk(frame);

	// check if the snrMapping is above the Decider's specific threshold,
	// i.e. the Decider has received it correctly
	if (result->isSignalCorrect()) {
		DBG << "packet was received correctly, it is now handed to upper layer...\n";
		// go on with processing this AirFrame, send it to the Mac-Layer
		phy->sendUp(frame, result);
	}
	else {
		DBG << "packet was not received correctly, sending it as control message to upper layer\n";
		Mac80211Pkt* mac;
		mac = static_cast<Mac80211Pkt*>(frame->decapsulate());
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


void Decider80211p::changeFrequency(double freq) {
	centerFrequency = freq;
}

Decider80211p::~Decider80211p() {
	double totalTime = simTime().dbl() - myStartTime;
	phy->recordScalar("busyTime",myBusyTime/totalTime);
};
