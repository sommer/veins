/* -*- mode:c++ -*- ********************************************************
 * file:        IEEE802154A.cc
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
 * description: this class holds constants specified in IEEE 802.15.4A UWB-IR Phy
 ***************************************************************************/

#include "IEEE802154A.h"

// bit rate (850 kbps)
const int IEEE802154A::mandatory_bitrate = 850000;
// mandatory data symbol length (1.025 ms)
const_simtime_t IEEE802154A::mandatory_symbol = 0.00000102364; //102564
// 0.5 * mandatory_symbol (0.5 ms)
const_simtime_t IEEE802154A::mandatory_timeShift = 0.00000051282;
// mandatory pulse duration ( = 1 / bandwidth = 2 ns)
const_simtime_t IEEE802154A::mandatory_pulse = 0.000000002003203125;
// burst duration (32 ns)
const_simtime_t IEEE802154A::mandatory_burst = 0.00000003205;
// number of consecutive pulses forming a burst
const int IEEE802154A::mandatory_pulses_per_burst = 16;
// Center frequency of band 3 in UWB lower band (500 MHz wide channel)
const double IEEE802154A::mandatory_centerFreq = 4498; // MHz
// default sync preamble length
const_simtime_t IEEE802154A::mandatory_preambleLength = 0.0000715; // Tpre=71.5 µs
// Total triangular pulse peak energy in mW (0 dBm / 50 MHz over 500 MHz)
const double IEEE802154A::maxPulse = 10;
// Duration of a sync preamble symbol
const_simtime_t IEEE802154A::Tpsym = 0.001 * 0.001 * 0.001 * 993.6; // 993.6 ns

const_simtime_t IEEE802154A::tFirstSyncPulseMax = IEEE802154A::mandatory_pulse/2;

// Ci values
const short IEEE802154A::C31[8][31] = {
// C1
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0 },
		// C2
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0 },
		// C3
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0 },
		// C4
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0 },
		// C5
		{ -1, 0, +1, -1, 0, 0, +1, +1, +1, -1, +1, 0, 0, 0, -1, +1, 0, +1, +1,
				+1, 0, -1, 0, +1, 0, 0, 0, 0, -1, 0, 0 },
		// C6
		{ +1, +1, 0, 0, +1, 0, 0, -1, -1, -1, +1, -1, 0, +1, +1, -1, 0, 0, 0,
				+1, 0, +1, 0, -1, +1, 0, +1, 0, 0, 0, 0 },
		// C7
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0 },
		// C8
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0 },

};

const short IEEE802154A::shortSFD[8] = { 0, 1, 0, -1, 1, 0, 0, -1 };

short IEEE802154A::s_array[maxS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0 };
int IEEE802154A::last_s = 15;


int IEEE802154A::psduLength = 0;

IEEE802154A::config IEEE802154A::cfg = { 3, NOMINAL_16_M, NON_RANGING, PSR_64,
		850000, 16, mandatory_symbol, mandatory_timeShift, mandatory_pulse,
		mandatory_burst, mandatory_preambleLength, 4498 };

void IEEE802154A::setPSDULength(int _psduLength) {
	assert(_psduLength < IEEE802154A::MaxPSDULength);
	IEEE802154A::psduLength = _psduLength;
}

IEEE802154A::signalAndData IEEE802154A::generateIEEE802154AUWBSignal(
		simtime_t signalStart) {
	// 96 R-S parity bits, the 2 symbols phy header is not modeled as it includes its own parity bits
	// and is thus very robust
	unsigned int nbBits = IEEE802154A::psduLength * 8 + 96;
	simtime_t signalLength = IEEE802154A::mandatory_preambleLength;
	signalLength += static_cast<double> (nbBits)
			* IEEE802154A::mandatory_symbol;
	Signal* s = new Signal(signalStart, signalLength);
	vector<bool>* bitValues = new vector<bool> ();

	signalAndData res;
	int bitValue;
	// data start time relative to signal->getSignalStart();
	simtime_t dataStart = IEEE802154A::mandatory_preambleLength; // = Tsync + Tsfd
	TimeMapping<Linear>* mapping = new TimeMapping<Linear> ();
	Argument* arg = new Argument();
	setBitRate(s);

	generateSyncPreamble(mapping, arg);
	generateSFD(mapping, arg);
	//generatePhyHeader(mapping, arg);

	// generate bit values and modulates them according to
	// the IEEE 802.15.4A specification
	simtime_t symbolStart = dataStart;
	simtime_t burstPos;
	for (unsigned int burst = 0; burst < nbBits; burst++) {
		bitValue = intuniform(0, 1, 0);
		bitValues->push_back(static_cast<bool>(bitValue));
		burstPos = symbolStart + bitValue*IEEE802154A::mandatory_timeShift + getHoppingPos(burst)*IEEE802154A::mandatory_burst;
		generateBurst(mapping, arg, burstPos, +1);
		symbolStart = symbolStart + IEEE802154A::mandatory_symbol;
	}
	//assert(uwbirMacPkt->getBitValuesArraySize() == dataLength);
	//assert(bitValues->size() == nbBits);

	// associate generated pulse energies to the signal
	s->setTransmissionPower(mapping);
	delete arg;

	res.first = s;
	res.second = bitValues;
	return res;
}

void IEEE802154A::generateSyncPreamble(Mapping* mapping, Argument* arg) {
	// NSync repetitions of the Si symbol
	for (short n = 0; n < NSync; n = n + 1) {
		for (short pos = 0; pos < CLength; pos = pos + 1) {
			if (C31[Ci - 1][pos] != 0) {
				arg->setTime(n * Tpsym + pos * IEEE802154A::spreadingdL
						* IEEE802154A::mandatory_pulse);
				generatePulse(mapping, arg, C31[Ci - 1][pos],
						IEEE802154A::maxPulse, IEEE802154A::mandatory_pulse);
			}
		}
	}
}

void IEEE802154A::generateSFD(Mapping* mapping, Argument* arg) {
	double sfdStart = NSync * Tpsym;
	for (short n = 0; n < 8; n = n + 1) {
		if (IEEE802154A::shortSFD[n] != 0) {
			for (short pos = 0; pos < CLength; pos = pos + 1) {
				if (C31[Ci - 1][pos] != 0) {
					arg->setTime(sfdStart + n * Tpsym + pos * IEEE802154A::spreadingdL
							* IEEE802154A::mandatory_pulse);
					generatePulse(mapping, arg, C31[Ci - 1][pos] * shortSFD[n]);
				}
			}
		}
	}
}

void IEEE802154A::generatePhyHeader(Mapping* mapping, Argument* arg) {
	// not implemented
}

void IEEE802154A::generatePulse(Mapping* mapping, Argument* arg,
		short polarity, double peak, simtime_t chip) {
	assert(polarity == -1 || polarity == +1);
	mapping->setValue(*arg, 0);
	arg->setTime(arg->getTime() + chip / 2);
	// Maximum point at symbol half (triangular pulse)
	mapping->setValue(*arg, peak * polarity);
	arg->setTime(arg->getTime() + chip / 2);
	mapping->setValue(*arg, 0);
}

void IEEE802154A::generateBurst(Mapping* mapping, Argument* arg,
		simtime_t burstStart, short polarity) {
	assert(burstStart < IEEE802154A::mandatory_preambleLength+(IEEE802154A::MaxPSDULength*8+96+2)*IEEE802154A::mandatory_symbol);
	// 1. Start point = zeros
	simtime_t offset = burstStart;
	for (int pulse = 0; pulse < IEEE802154A::mandatory_pulses_per_burst; pulse++) {
		arg->setTime(offset);
		generatePulse(mapping, arg, 1);
		offset = offset + IEEE802154A::mandatory_pulse;
	}
}

void IEEE802154A::setBitRate(Signal* s) {
	Argument arg = Argument();
	// set a constant value for bitrate
	TimeMapping<Linear>* bitrate = new TimeMapping<Linear> ();
	arg.setTime(0); // relative to signal->getSignalStart()
	bitrate->setValue(arg, 1 / IEEE802154A::mandatory_symbol);
	arg.setTime(s->getSignalLength());
	bitrate->setValue(arg, 1 / IEEE802154A::mandatory_symbol);
	s->setBitrate(bitrate);
}

simtime_t IEEE802154A::getThdr() {
	switch (IEEE802154A::cfg.channel) {
	default:
		switch (IEEE802154A::cfg.prf) {
		case NOMINAL_4_M:
			//error("This optional mode is not implemented.");
			return 0;
			break;
		case NOMINAL_16_M:
			return 16.4E-6;
		case NOMINAL_64_M:
			return 16.8E-6;
		case PRF_OFF:
			return 0;
		}
	}
	return 0;
}

int IEEE802154A::s(int n) {

	assert(n < maxS);

	for (; last_s < n; last_s = last_s + 1) {
		// compute missing values as necessary
		s_array[last_s] = (s_array[last_s - 14] + s_array[last_s - 15]) % 2;
	}
	assert(s_array[n] == 0 || s_array[n] == 1);
	return s_array[n];

}

int IEEE802154A::getHoppingPos(int sym) {
	// Warning: only valid for the mandatory mode
	//int m = 3;
	int Ncpb = 16;
	int pos = 0;
	pos = s(sym * Ncpb) + 2 * s(1 + sym * Ncpb) + 4 * s(2 + sym * Ncpb);
	assert(pos > -1 && pos < 8);
	return pos;
}

simtime_t IEEE802154A::getPhyMaxFrameDuration() {
	simtime_t phyMaxFrameDuration = 0;
	simtime_t TSHR, TPHR, TPSDU, TCCApreamble;
	TSHR = IEEE802154A::getThdr();
	TPHR = IEEE802154A::getThdr();
	phyMaxFrameDuration = phyMaxFrameDuration + TSHR;
	return phyMaxFrameDuration;
}
