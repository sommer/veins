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

	// get the receiving power of the Signal at start-time and center frequency
	Signal& signal = frame->getSignal();

	Argument start(DimensionSet::timeFreqDomain);
	start.setTime(signal.getReceptionStart());
	start.setArgValue(Dimension::frequency_static(), centerFrequency);

	double recvPower = signal.getReceivingPower()->getValue(start);

	if (recvPower < sensitivity) {
		return notAgain;
	}
	else {
		signalStates[frame] = EXPECT_END;

		setChannelIdleStatus(false);

		if (phy11p->getRadioState() == Radio::TX) {
			signalStates[frame] = EXPECT_END;
			frame->setBitError(true);
			DBG_D11P << "AirFrame: " << frame->getId() << " (" << recvPower << ") received, while already sending. Setting BitErrors to true" << endl;
		}
		else {

			DBG_D11P << "AirFrame: " << frame->getId() << " with (" << recvPower << " > " << sensitivity << ") -> Trying to receive AirFrame." << endl;

			//channel turned busy
			//measure communication density
			myBusyTime += signal.getDuration().dbl();
		}
		return signal.getReceptionEnd();
	}
}

int Decider80211p::getSignalState(AirFrame* frame) {

	if (signalStates.find(frame) == signalStates.end()) {
		return NEW;
	}
	else {
		return signalStates[frame];
	}
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
		DBG_D11P << "Packet is fine! We can decode it" << std::endl;
		result = new DeciderResult80211(true, payloadBitrate, snirMin);
	}
	else {
		DBG_D11P << "Packet has bit Errors. Lost " << std::endl;
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


bool Decider80211p::cca(simtime_t_cref time, AirFrame* exclude) {
	AirFrameVector airFrames;

	// collect all AirFrames that intersect with [start, end]
	getChannelInfo(time, time, airFrames);

	Mapping* resultMap = MappingUtils::createMapping(Argument::MappedZero, DimensionSet::timeDomain);

	//add thermal noise
	ConstMapping* thermalNoise = phy->getThermalNoise(time, time);
	if (thermalNoise) {
		Mapping* tmp = resultMap;
		resultMap = MappingUtils::add(*resultMap, *thermalNoise);
		delete tmp;
	}

	// otherwise, iterate over all AirFrames (except exclude)
	// and sum up their receiving-power-mappings
	for (AirFrameVector::const_iterator it = airFrames.begin(); it != airFrames.end(); ++it) {
		if (*it == exclude) {
			continue;
		}
		// the vector should not contain pointers to 0
		assert(*it != 0);

		// if iterator points to exclude (that includes the default-case 'exclude == 0')
		// then skip this AirFrame

		// otherwise get the Signal and its receiving-power-mapping
		Signal& signal = (*it)->getSignal();

		// backup pointer to result map
		// Mapping* resultMapOld = resultMap;

		// TODO1.1: for testing purposes, for now we don't specify an interval
		// and add the Signal's receiving-power-mapping to resultMap in [start, end],
		// the operation Mapping::add returns a pointer to a new Mapping

		const ConstMapping* const recvPowerMap = signal.getReceivingPower();
		assert(recvPowerMap);

		// Mapping* resultMapNew = Mapping::add( *(signal.getReceivingPower()), *resultMap, start, end );

		Mapping* resultMapNew = MappingUtils::add(*recvPowerMap, *resultMap, Argument::MappedZero);

		// discard old mapping
		delete resultMap;
		resultMap = resultMapNew;
		resultMapNew = 0;
	}

	Argument min(DimensionSet::timeFreqDomain);
	min.setTime(time);
	min.setArgValue(Dimension::frequency_static(), centerFrequency - 5e6);

	DBG_D11P << MappingUtils::findMin(*resultMap, min, min) << " > " << sensitivity << " = " << (bool)(MappingUtils::findMin(*resultMap, min, min) > sensitivity) << std::endl;
	bool isChannelIdle = MappingUtils::findMin(*resultMap, min, min) < sensitivity;
	delete resultMap;
	return isChannelIdle;
}


simtime_t Decider80211p::processSignalEnd(AirFrame* frame) {
	// here the Signal is finally processed

	bool whileSending = false;

	//remove this frame from our current signals
	signalStates.erase(frame);

	DeciderResult* result;


	if (frame->hasBitError() || phy11p->getRadioState() == Radio::TX) {
		//this frame was received while sending
		whileSending = true;
		result = new DeciderResult80211(false,0,0);
	}
	else {
		// check if the snrMapping is above the Decider's specific threshold,
		// i.e. the Decider has received it correctly
		result = checkIfSignalOk(frame);
	}

	if (result->isSignalCorrect()) {
		DBG_D11P << "packet was received correctly, it is now handed to upper layer...\n";
		// go on with processing this AirFrame, send it to the Mac-Layer
		phy->sendUp(frame, result);
	}
	else {
		DBG_D11P << "packet was not received correctly, sending it as control message to upper layer\n";
		if (whileSending) {
			phy->sendControlMsgToMac(new cMessage("Error",RECWHILESEND));
		}
		else {
			phy->sendControlMsgToMac(new cMessage("Error",BITERROR));
		}
		delete result;
	}

	if (phy11p->getRadioState() == Radio::TX) {
		DBG_D11P << "I'm currently sending\n";
	}
	//check if channel is idle now
	else if (cca(simTime(), frame) == false) {
		DBG_D11P << "Channel not yet idle!\n";
	}
	else {
		//might have been idle before (when the packet rxpower was below sens)
		if (isChannelIdle != true) {
			DBG_D11P << "Channel idle now!\n";
			setChannelIdleStatus(true);
		}
	}
	return notAgain;
}

void Decider80211p::setChannelIdleStatus(bool isIdle) {
	isChannelIdle = isIdle;
	channelStateChanged();
	if (isIdle) phy->sendControlMsgToMac(new cMessage("ChannelStatus",Mac80211pToPhy11pInterface::CHANNEL_IDLE));
	else phy->sendControlMsgToMac(new cMessage("ChannelStatus",Mac80211pToPhy11pInterface::CHANNEL_BUSY));
}

void Decider80211p::changeFrequency(double freq) {
	centerFrequency = freq;
}

Decider80211p::~Decider80211p() {
	double totalTime = simTime().dbl() - myStartTime;
	phy->recordScalar("busyTime",myBusyTime/totalTime);
};
