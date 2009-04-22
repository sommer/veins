/* -*- mode:c++ -*- ********************************************************
 * file:        UWBIREnergyDetectionDeciderV2.cc
 *
 * author:      Jerome Rousselot <jerome.rousselot@csem.ch>
 *
 * copyright:   (C) 2008 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 * 				Systems Engineering
 *              Real-Time Software and Networking
 *              Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 * description: this Decider models a non-coherent energy-detection receiver
 ***************************************************************************/

#include "UWBIREnergyDetectionDeciderV2.h"
#include "UWBIRPhyLayer.h"


simtime_t UWBIREnergyDetectionDeciderV2::processSignal(AirFrame* frame) {
	Signal* s = &frame->getSignal();
	map<Signal*, int>::iterator it = currentSignals.find(s);

	if (it == currentSignals.end()) {
		return handleNewSignal(s);
	} else {
		switch (it->second) {
		case HEADER_OVER:
			return handleHeaderOver(it);
		case SIGNAL_OVER:
			return handleSignalOver(it, frame);
		default:
			break;
		}
	}
	//we should never get here!
	assert(false);
	return 0;
}

simtime_t UWBIREnergyDetectionDeciderV2::handleNewSignal(Signal* s) {

	int currState = uwbiface->getRadioState();
	if (tracking == 0 && currState == Radio::SYNC) {
		  simtime_t endOfHeader = s->getSignalStart()
				+ IEEE802154A::mandatory_preambleLength;
		  currentSignals[s] = HEADER_OVER;
		  assert(endOfHeader> 0);
		  return endOfHeader;
	} else {
		// we are already tracking an airframe, this is noise
		// or we are transmitting, switching, or sleeping
		currentSignals[s] = SIGNAL_OVER;
		simtime_t endOfSignal = s->getSignalStart() + s->getSignalLength();
		assert(endOfSignal> 0);
		return endOfSignal;
	}

}

simtime_t UWBIREnergyDetectionDeciderV2::handleHeaderOver(
		map<Signal*, int>::iterator& it) {

	int currState = uwbiface->getRadioState();
	Signal* s = it->first;

	if (tracking == 0 && currState == Radio::SYNC) {
		// We are not tracking a signal currently.
		// Can we synchronize on this one ?

		bool isSyncSignalHigherThanThreshold;
		if(syncAlwaysSucceeds) {
			isSyncSignalHigherThanThreshold = true;
		} else {
			isSyncSignalHigherThanThreshold = attemptSync(s);
		}

		packet.setNbSyncAttempts(packet.getNbSyncAttempts() + 1);

		if(isSyncSignalHigherThanThreshold) {
			nbSuccessfulSyncs = nbSuccessfulSyncs + 1;
			tracking = s;
			synced = true;
			currentSignals[s] = SIGNAL_OVER;
			uwbiface->switchRadioToRX();
			packet.setNbSyncSuccesses(packet.getNbSyncSuccesses() + 1);
		} else {
			nbFailedSyncs = nbFailedSyncs + 1;
		}
		utility->publishBBItem(catUWBIRPacket, &packet, -1); // scope = all == host

	}
	// in any case, look at that frame again when it is finished
	it->second = SIGNAL_OVER;
	simtime_t startOfSignal = it->first->getSignalStart();
	simtime_t lengthOfSignal = it->first->getSignalLength();
	simtime_t endOfSignal = startOfSignal + lengthOfSignal;
	assert(endOfSignal> 0);
	return endOfSignal;
}

bool UWBIREnergyDetectionDeciderV2::attemptSync(Signal* s) {
	double value;
	//double meanSyncPreambleEnergy = 0;
	//int nbValues = 0;
	ConstMapping* power = s->getReceivingPower();
	ConstMappingIterator* mIt = power->createConstIterator();

	vector<AirFrame*> syncVector;
	// Retrieve all potentially colliding airFrames
	phy->getChannelInfo(s->getSignalStart(), simTime(), syncVector);

	if(syncVector.size() > 1) {
		// do not accept interferers
		return false;
	}
	Argument posFirstPulse(IEEE802154A::tFirstSyncPulseMax);
	mIt->jumpTo(posFirstPulse);
	value = std::abs(mIt->getValue());
    syncThresholds.record(value);
    if(value/noiseLevel > syncThreshold) {
    	return true;
    } else {
    	return false;
    }
}

simtime_t UWBIREnergyDetectionDeciderV2::handleSignalOver(
		map<Signal*, int>::iterator& it, AirFrame* frame) {
	if (it->first == tracking) {
		vector<bool>* receivedBits = new vector<bool>();
		decodePacket(it->first, receivedBits);
		// we cannot compute bit error rate here
		// so we send the packet to the MAC layer which will compare receivedBits
		// with the actual bits sent.
		UWBIRDeciderResult * result = new UWBIRDeciderResult(true, receivedBits);
		phy->sendUp(frame, result);
		currentSignals.erase(it);
		tracking = 0;
		synced = false;
		uwbiface->switchRadioToSync();
		return -1;
	} else {
		// reached end of noisy signal
		currentSignals.erase(it);
		return -1;
	}
}

void UWBIREnergyDetectionDeciderV2::decodePacket(Signal* signal,
		vector<bool> * receivedBits) {

	simtime_t now, offset;
	//long double noiseLevel;
	simtime_t aSymbol, shift, burst;

	// Retrieve all potentially colliding airFrames
	phy->getChannelInfo(signal->getSignalStart(), signal->getSignalStart()
			+ signal->getSignalLength(), airFrameVector);

	for (airFrameIter = airFrameVector.begin(); airFrameIter
			!= airFrameVector.end(); ++airFrameIter) {
		Signal & aSignal = (*airFrameIter)->getSignal();
		offsets.push_back(signal->getSignalStart() - aSignal.getSignalStart());
		ConstMapping* currPower = aSignal.getReceivingPower();
		receivingPowers.push_back(currPower);
		if (aSignal.getSignalStart() == signal->getSignalStart()
				&& aSignal.getSignalLength() == signal->getSignalLength()) {
			// Why is this false ??
			//assert(signalPower == currPower);
			signalPower = currPower;
		}
	}

	// all times are relative to signal->getSignalStart() !
	offset = IEEE802154A::mandatory_preambleLength;
	shift = IEEE802154A::mandatory_timeShift;
	aSymbol = IEEE802154A::mandatory_symbol;
	burst = IEEE802154A::mandatory_burst;
	//double noise_interferences;
	//bool morePulses;
	//noiseLevel = kB500M*temperature;
	//const double centerFreq = IEEE802154A::mandatory_centerFreq;
	now = IEEE802154A::mandatory_preambleLength + IEEE802154A::mandatory_pulse / 2;
	std::pair<double, double> energyZero, energyOne;

	// debugging information (start)
	if (trace) {
		/*
		signalLengths.recordWithTimestamp(signal->getSignalStart(),
					signal->getSignalLength()); */
		simtime_t prev = 0;
		ConstMappingIterator* iteratorDbg = signalPower->createConstIterator();
		int nbItems = 0;
		iteratorDbg->jumpToBegin();
		while (iteratorDbg->hasNext()) {
			nbItems = nbItems + 1;
			iteratorDbg->next();

			receivedPulses.recordWithTimestamp(signal->getSignalStart()
												+ iteratorDbg->getPosition().getTime(),
											   signalPower->getValue(iteratorDbg->getPosition()));
			prev = iteratorDbg->getPosition().getTime();
			simtime_t t = signal->getSignalStart() + prev;
			/*
			if(std::abs(signalPower->getValue(iteratorDbg->getPosition())) > 0) {
				EV << "nbItems=" << nbItems << ", t= " << t <<  ", value=maxPulse." << endl;
			} else {
			EV << "nbItems=" << nbItems << ", t= " << t <<  ", value=" << signalPower->getValue(iteratorDbg->getPosition()) << "." << endl;
			}
			*/
		}
		delete iteratorDbg;
	}
	// debugging information (end)


	// Loop to decode each bit value
	for (int symbol = 0; IEEE802154A::mandatory_preambleLength + symbol
			* aSymbol < signal->getSignalLength(); symbol++) {

		int hoppingPos = IEEE802154A::getHoppingPos(symbol);
		if(trace) {
		  timeHoppings.record(hoppingPos);
		}
		now = now + IEEE802154A::getHoppingPos(symbol)*IEEE802154A::mandatory_burst;
		// sample in window zero
		energyZero = integrateWindow(symbol, now, burst, noiseLevel, signal);
		// sample in window one
		now = now + shift;
		energyOne = integrateWindow(symbol, now, burst, noiseLevel, signal);

		//noise_interferences = energyZero.second + energyOne.second;
		//long double snir_symbol = ( energyZero.first + energyOne.first ) / noise_interferences;
		//symbolsSNRs.record(snir_symbol);
		//decode
		int decodedBit;
		double threshold = min(energyZero.second, energyOne.second) / max(
				energyZero.second, energyOne.second);

		if (stats) {
			allThresholds = allThresholds + threshold;
			nbSymbols = nbSymbols + 1;
		}

		if (threshold < sensitivity) {
			int randomValue = intuniform(0, 1);
			if (randomValue == 0) {
				decodedBit = 0;
			} else {
				decodedBit = 1;
			}
			nbRandomBits = nbRandomBits + 1;
		} else {
			if (energyZero.second > energyOne.second) {
				decodedBit = 0;
			} else {
				decodedBit = 1;
			}
		}

		receivedBits->push_back(static_cast<bool>(decodedBit));

		// update state variables
		now = offset + (symbol + 1) * aSymbol + IEEE802154A::mandatory_pulse
				/ 2;

	}

	// free memory
	/*
	 vector<Mapping*>::iterator mappingIter;
	 for (mappingIter = pulsesIters.begin(); mappingIter != pulsesIters.end(); ++mappingIter) {
	 delete *mappingIter;
	 }
	 */
	receivingPowers.clear();
	airFrameVector.clear();
	offsets.clear();
	//delete receivingPower;
}

pair<double, double> UWBIREnergyDetectionDeciderV2::integrateWindow(int symbol,
		simtime_t now, simtime_t burst, double noiseLevel, Signal* signal) {
	std::pair<double, double> energy;
	energy.first = 0; // stores energy contribution from tracked signal
	energy.second = 0; // stores total captured window energy
	vector<ConstMapping*>::iterator mappingIter;
	Argument arg;
	simtime_t windowEnd = now + burst;
	//double amplitudes;
	//double sigAmplitudes;


	// Version 1.5
	// Triangular pulses
	// we sample at each pulse peak
	// get the interpolated values of amplitude for each interferer
	// and add these to the peak with a random phase

	// we sample one point per pulse
	// caller has already set our time reference ("now") at the peak of the pulse
	for (; now < windowEnd; now += IEEE802154A::mandatory_pulse) {
		double sampling = noiseLevel;
		double signalValue = 0;
		arg.setTime(now);
		int currSig = 0;
		// consider all interferers at this point in time
		for (airFrameIter = airFrameVector.begin(); airFrameIter
				!= airFrameVector.end(); ++airFrameIter) {

			Signal & aSignal = (*airFrameIter)->getSignal();
			ConstMapping* currPower = aSignal.getReceivingPower();
			arg.setTime(now + offsets.at(currSig));
			double measure = currPower->getValue(arg);
			if (currPower == signalPower) {
				signalValue += measure;
				sampling += measure;
			} else {
				// random phase for interferer
				sampling += measure * intuniform(-1, 1);
			}
			++currSig;
		}
		double val = signalValue * IEEE802154A::mandatory_pulse / 2; // signalValue*signalValue * samplingInterval;
		energy.first = energy.first + pow(val, 2);
		//energy.second += sampling * sampling * (IEEE802154A::mandatory_pulse) * (IEEE802154A::mandatory_pulse) / 4;
		// convert from mW to Voltpeak:
		double voltPeak = 2 * 50 * sampling; // 50 Ohm
		energy.second = energy.second + pow(voltPeak, 2);
		//energy.second += sampling * sampling * IEEE802154A::mandatory_pulse * IEEE802154A::mandatory_pulse / 4;
	} // consider next point in time
	// End version 1.5


	/*
	 // new integration method based on discussion with epfl team
	 mappingIter = pulsesIters.begin();
	 // iterate over each signal and interferer
	 sigAmplitudes = 0;
	 while(mappingIter != pulsesIters.end()) {
	 // iterate over all pulses of current signal/interferer
	 while( (*mappingIter)->hasNext() &&
	 (*mappingIter)->getNextPosition().getTime() < windowEnd ) {
	 (*mappingIter)->next();
	 sigAmplitudes = sigAmplitudes + (*mappingIter)->getValue();
	 }
	 if( *mappingIter == signalPower ) {
	 energy.first = sigAmplitudes * IEEE802154A::mandatory_pulse / 2;
	 energy.first = energy.first * energy.first;
	 }  else {
	 // random phase of interferer
	 sigAmplitudes = sigAmplitudes * genk_intuniform(0, -1, 1);
	 }
	 amplitudes = amplitudes + sigAmplitudes;
	 }

	 energy.second = amplitudes * IEEE802154A::mandatory_pulse / 2;
	 energy.second = energy.second * energy.second;
	 */
	// end of new integration method

	return energy;
}

simtime_t UWBIREnergyDetectionDeciderV2::handleChannelSenseRequest(
		ChannelSenseRequest* request) {
	if (channelSensing) {
		// send back the channel state
		request->setResult(new ChannelState(synced, 0)); // bogus rssi value (0)
		phy->sendControlMsg(request);
		channelSensing = false;
		return -1; // do not call me back ; I have finished
	} else {
		channelSensing = true;
		return -1; //phy->getSimTime() + request->getSenseDuration();
	}
}


