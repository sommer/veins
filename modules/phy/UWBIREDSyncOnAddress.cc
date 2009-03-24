/* -*- mode:c++ -*- ********************************************************
 * file:        UWBIREDSyncOnAddress.cc
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
 * 				that always synchronizes successfully on frames coming from
 * 				a particular node.
 ***************************************************************************/

#include "UWBIREDSyncOnAddress.h"
#include "UWBIREnergyDetectionDeciderV2.h"

UWBIREDSyncOnAddress::UWBIREDSyncOnAddress(DeciderToPhyInterface* iface,
				UWBIRPhyLayer* _uwbiface, double _sensitivity,
				double _syncThreshold, bool _syncAlwaysSucceeds, bool _stats,
				bool _trace, int _addr) :
					UWBIREnergyDetectionDeciderV2(iface, _uwbiface, _sensitivity,
						_syncThreshold, _syncAlwaysSucceeds, _stats, _trace), syncAddress(_addr) {

};


bool UWBIREDSyncOnAddress::attemptSync(Signal* s) {
	cMessage* encaps = currFrame->getEncapsulatedMsg();
	assert(static_cast<MacPkt*>(encaps));
	MacPkt* macPkt = static_cast<MacPkt*>(encaps);

	return (macPkt->getSrcAddr()==syncAddress);
};

simtime_t UWBIREDSyncOnAddress::processSignal(AirFrame* frame) {
	currFrame = frame;
	return UWBIREnergyDetectionDeciderV2::processSignal(frame);
};


