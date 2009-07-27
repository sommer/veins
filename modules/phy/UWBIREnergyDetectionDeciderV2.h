/* -*- mode:c++ -*- ********************************************************
 * file:        UWBIREnergyDetectionDeciderV2.h
 *
 * author:      Jerome Rousselot <jerome.rousselot@csem.ch>
 *
 * copyright:   (C) 2008-2009 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
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
 * description: this Decider models an energy-detection receiver with soft-decision
 ***************************************************************************/

#ifndef _UWBIRENERGYDETECTIONDECIDERV2_H
#define	_UWBIRENERGYDETECTIONDECIDERV2_H

#include <vector>
#include <map>
#include <math.h>

#include "Signal_.h"
#include "Mapping.h"
#include "AirFrame_m.h"
#include "Decider.h"
#include "UWBIRDeciderResult.h"
#include "AlohaMacLayer.h"
#include "IEEE802154A.h"
#include "UWBIRPacket.h"
#include "BaseUtility.h"
//#include "UWBIRRadio.h"
//#include "PhyUtils.h"

using namespace std;

#define PI 3.1415926

class UWBIRPhyLayer;


class UWBIREnergyDetectionDeciderV2: public Decider {
private:
	bool trace, stats;
	long nbRandomBits;
	long nbFailedSyncs, nbSuccessfulSyncs;
	double nbSymbols, allThresholds;
	double vsignal2, vnoise2, snirs, snirEvals, pulseSnrs;
	double packetSNIR, packetSignal, packetNoise, packetSamples;

protected:
	double syncThreshold;
	bool syncAlwaysSucceeds;
	UWBIRPacket packet;
	int catUWBIRPacket;
	BaseUtility* utility;
	double epulseAggregate, enoiseAggregate;

public:
	// Boltzmann constant multiplied by 500 MHz (signal bandwidth) in mJ.K-1 !
	const static double kB500M = 5 * 1.38E-12; // mW/K
	const static int temperature = 293; // 20 Celsius degrees
	const static double noiseVariance = 404.34E-12;  // v²=s²=4kb T R B
	const static double Ptx = 37.06E-6; // radiated power at origin (-41.3 dBm/MHz over 500 MHz in Watts)
	const static double peakPulsePower = 37.06E-6; //1E-3; // peak instantaneous power of the transmitted pulse (A=0.6V) : 7E-3 W. But peak limit is 0 dBm
	const static double peakVoltage = 0.6; // peak voltage of the triangular pulse to reach Ptx
	const static double resistor = 50; // 50 Ohms
	const static double lambda = 0.04;// center frequency wavelength
	const static double Aiso = 127.32E-6; // Aperture of the ideal isotropic antenna (lambda²/4Pi)

	UWBIREnergyDetectionDeciderV2(DeciderToPhyInterface* iface,
			UWBIRPhyLayer* _uwbiface,
			double _syncThreshold, bool _syncAlwaysSucceeds, bool _stats,
			bool _trace) :
		Decider(iface), trace(_trace),
				stats(_stats), nbRandomBits(0), nbFailedSyncs(0),
				nbSuccessfulSyncs(0), nbSymbols(0), syncThreshold(_syncThreshold),
				syncAlwaysSucceeds(_syncAlwaysSucceeds), uwbiface(_uwbiface), tracking(0),
				channelSensing(false), synced(false), vsignal2(0), vnoise2(0), snirEvals(0), pulseSnrs(0) {

		zerosEnergies.setName("ZerosEnergies");
		onesEnergies.setName("OnesEnergies");
		signalLengths.setName("signalLengths");
		receivedPulses.setName("receivedPulses");
		syncThresholds.setName("syncThresholds");
		timeHoppings.setName("timeHoppings");
		decisionVariable.setName("decisionVariable");
		packetDecisionAvg.setName("decisionAvg");
		ebN0.setName("EbN0");
		pulseSINR.setName("sinr");
		noisePower.setName("noise");
		receivedPower.setName("Prx");
		utility = iface->getUtility();
		catUWBIRPacket = utility->getCategory(&packet);
	};

	virtual simtime_t processSignal(AirFrame* frame);

	long getNbRandomBits() {
		return nbRandomBits;
	};

	double getAvgThreshold() {
		if (nbSymbols > 0)
			return allThresholds / nbSymbols;
		else
			return 0;
	};

	long getNbFailedSyncs() {
		return nbFailedSyncs;
	};

	long getNbSuccessfulSyncs() {
		return nbSuccessfulSyncs;
	};

	double getNoiseValue() {
		 return normal(0, noiseVariance);
	}

	double getInducedPowerFromEField(double Efield) {
		double inducedPower = 0;
		double intensity = pow(Efield, 2) / 120*PI;
		inducedPower = intensity * Aiso;
		return inducedPower;
	}

protected:
	map<Signal*, int> currentSignals;
	cOutVector zerosEnergies, onesEnergies, decisionVariable, packetDecisionAvg, signalLengths,
			receivedPulses;
	cOutVector syncThresholds, timeHoppings;
	cOutVector ebN0;
	cOutVector pulseSINR;
    cOutVector noisePower, receivedPower;

	UWBIRPhyLayer* uwbiface;
	Signal* tracking;
	enum {
		FIRST, HEADER_OVER, SIGNAL_OVER
	};

	bool channelSensing;
	bool synced;

	vector<ConstMapping*> receivingPowers;
	ConstMapping* signalPower; // = signal->getReceivingPower();
	// store relative offsets between signals starts
	vector<simtime_t> offsets;
	vector<AirFrame*> airFrameVector;
	// Create an iterator for each potentially colliding airframe
	vector<AirFrame*>::iterator airFrameIter;

	typedef ConcatConstMapping<std::multiplies<double> > MultipliedMapping;

	void decodePacket(Signal* signal, vector<bool> * receivedBits);
	simtime_t handleNewSignal(Signal* s);
	simtime_t handleHeaderOver(map<Signal*, int>::iterator& it);
	virtual bool attemptSync(Signal* signal);
	simtime_t
			handleSignalOver(map<Signal*, int>::iterator& it, AirFrame* frame);
	// first value is energy from signal, other value is total window energy
	pair<double, double> integrateWindow(int symbol, simtime_t now,
			simtime_t burst, Signal* signal);

	simtime_t handleChannelSenseRequest(ChannelSenseRequest* request);

};

#endif	/* _UWBIRENERGYDETECTIONDECIDERV2_H */

