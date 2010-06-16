/*
 * Decider802154Narrow.cc
 *
 *  Created on: 11.02.2009
 *      Author: karl wessel
 */

#include "Decider802154Narrow.h"
#include "DeciderResult802154Narrow.h"
#include "Consts802154.h"
#include <MacPkt_m.h>
#include <PhyToMacControlInfo.h>

bool Decider802154Narrow::syncOnSFD(AirFrame* frame) {
	double BER;
	double sfdErrorProbability;

	BER = evalBER(frame);
	sfdErrorProbability = 1.0 - pow((1.0 - BER), sfdLength);

	return sfdErrorProbability < uniform(0, 1, 0);
}

double Decider802154Narrow::evalBER(AirFrame* frame) {
	Signal& signal = frame->getSignal();

	simtime_t time = MappingUtils::post(phy->getSimTime());
	double rcvPower = signal.getReceivingPower()->getValue(Argument(time));

	ConstMapping* noise = calculateRSSIMapping(time, time, frame);

	double noiseLevel = noise->getValue(Argument(time));

	delete noise;

	return std::max(0.5 * exp(-rcvPower / (2 * noiseLevel)),
			DEFAULT_BER_LOWER_BOUND);
}

simtime_t Decider802154Narrow::processNewSignal(AirFrame* frame) {
	//if we are already receiving another signal this is noise
	if(currentSignal.first != 0) {
		return notAgain;
	}

	if (!syncOnSFD(frame)) {
		return notAgain;
	}

	// store this frame as signal to receive and set state
	currentSignal.first = frame;
	currentSignal.second = EXPECT_END;

	//channel is busy now
	setChannelIdleStatus(false);

	//TODO: publish rssi and channel state

	Signal& s = frame->getSignal();
	return s.getSignalStart() + s.getSignalLength();
}

simtime_t Decider802154Narrow::processSignalEnd(AirFrame* frame)
{
	ConstMapping* snrMapping = calculateSnrMapping(frame);

	Signal& s = frame->getSignal();
	simtime_t start = s.getSignalStart();
	simtime_t end = start + s.getSignalLength();

	AirFrameVector channel;
	phy->getChannelInfo(start, end, channel);
	bool hasInterference = channel.size() > 1;

	if(hasInterference) {
		nbFramesWithInterference++;
	} else {
		nbFramesWithoutInterference++;
	}

	double bitrate = s.getBitrate()->getValue(Argument(start));

	double avgBER = 0;
	double bestBER = 0.5;
	double snirAvg = 0;
	bool noErrors = true;
	double ber;
	double errorProbability;

	simtime_t receivingStart = MappingUtils::post(start);
	ConstMappingIterator* iter = snrMapping->createConstIterator(Argument(receivingStart));
	double snirMin = iter->getValue();
	// Evaluate bit errors for each snr value
	// and stops as soon as we have an error.
	// TODO: add error correction code.

	simtime_t curTime = iter->getPosition().getTime();
	simtime_t snrDuration;
	while(curTime < end) {
		//get SNR for this interval
		double snr = iter->getValue();

		//determine end of this interval
		simtime_t nextTime = end;	//either the end of the signal...
		if(iter->hasNext()) {		//or the position of the next entry
			nextTime = std::min(iter->getNextPosition().getTime(), nextTime);
			iter->next();	//the iterator will already point to the next entry
		}

		if (noErrors) {
			snrDuration = nextTime - curTime;

			int nbBits = int (snrDuration.dbl() * bitrate);

			// non-coherent detection of m-ary orthogonal signals in an AWGN
			// Channel
			// Digital Communications, John G. Proakis, section 4.3.2
			// p. 212, (4.3.32)
			//  Pm = sum(n=1,n=M-1){(-1)^(n+1)choose(M-1,n) 1/(n+1) exp(-nkgamma/(n+1))}
			// Pb = 2^(k-1)/(2^k - 1) Pm


			ber = std::max(getBERFromSNR(snr), BER_LOWER_BOUND);
			avgBER = ber*nbBits;
			snirAvg = snirAvg + snr*snrDuration.dbl();

			if(ber < bestBER) {
				bestBER = ber;
			}
			errorProbability = 1.0 - pow((1.0 - ber), nbBits);
			noErrors = errorProbability < uniform(0, 1);
		}
		if (snr < snirMin)
			snirMin = snr;

		curTime = nextTime;
	}
	delete iter;
	delete snrMapping;

	avgBER = avgBER / frame->getBitLength();
	snirAvg = snirAvg / (end - start);
	double rssi = 10*log10(snirAvg);
	if (noErrors)
	{
		phy->sendUp(frame,
					new DeciderResult802154Narrow(true, bitrate, snirMin, avgBER, rssi));
	} else {
		MacPkt* mac = static_cast<MacPkt*> (frame->decapsulate());
		mac->setName("BIT_ERRORS");
		mac->setKind(PACKET_DROPPED);

		DeciderResult802154Narrow* result =
			new DeciderResult802154Narrow(false, bitrate, snirMin, avgBER, rssi);
		PhyToMacControlInfo* cInfo = new PhyToMacControlInfo(result);
		mac->setControlInfo(cInfo);
		phy->sendControlMsg(mac);

		if(hasInterference) {
			nbFramesWithInterferenceDropped++;
		} else {
			nbFramesWithoutInterferenceDropped++;
		}
	}

	//decoding is done
	currentSignal.first = 0;

	//channel is back idle
	setChannelIdleStatus(true);

	//TODO: publish rssi and channel state
	return notAgain;
}

double Decider802154Narrow::getBERFromSNR(double snr) {
	double ber = BER_LOWER_BOUND;

	if(modulation == "msk") {
		// non-coherent detection of binary signals in an AWGN Channel
		// Digital Communications, John G. Proakis, section 4.3.1
		// p.207, (4.3.19)
		ber = 0.5 * exp(-0.5 * snr);
	} else if (modulation == "FMUWB") {
		ber = 0.5 * exp(-0.5 * snr);
	}

	return std::max(ber, BER_LOWER_BOUND);
}


void Decider802154Narrow::finish() {

	// record scalars through the interface to the PHY-Layer
	phy->recordScalar("nbFramesWithInterference", nbFramesWithInterference);
	phy->recordScalar("nbFramesWithoutInterference", nbFramesWithoutInterference);
	phy->recordScalar("nbFramesWithInterferenceDropped", nbFramesWithInterferenceDropped);
	phy->recordScalar("nbFramesWithoutInterferenceDropped", nbFramesWithoutInterferenceDropped);
}
