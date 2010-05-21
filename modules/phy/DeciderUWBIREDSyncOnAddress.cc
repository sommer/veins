/* -*- mode:c++ -*- ********************************************************
 * file:        DeciderUWBIREDSyncOnAddress.cc
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
 * acknowledgment: this work was supported (in part) by the National Competence
 * 			    Center in Research on Mobile Information and Communication Systems
 * 				NCCR-MICS, a center supported by the Swiss National Science
 * 				Foundation under grant number 5005-67322.
 ***************************************************************************/

#include "DeciderUWBIREDSyncOnAddress.h"
#include "DeciderUWBIRED.h"

DeciderUWBIREDSyncOnAddress::DeciderUWBIREDSyncOnAddress(DeciderToPhyInterface* iface,
				PhyLayerUWBIR* _uwbiface,
				double _syncThreshold, bool _syncAlwaysSucceeds, bool _stats,
				bool _trace, int _addr, bool alwaysFailOnDataInterference) :
					DeciderUWBIRED(iface, _uwbiface,
						_syncThreshold, _syncAlwaysSucceeds, _stats, _trace, alwaysFailOnDataInterference),
						syncAddress(_addr) {

};


bool DeciderUWBIREDSyncOnAddress::attemptSync(Signal* s) {
	cMessage* encaps = currFrame->getEncapsulatedPacket();
	assert(static_cast<MacPkt*>(encaps));
	MacPkt* macPkt = static_cast<MacPkt*>(encaps);

	return (macPkt->getSrcAddr()==syncAddress);
};

simtime_t DeciderUWBIREDSyncOnAddress::processSignal(AirFrame* frame) {
	currFrame = frame;
	return DeciderUWBIRED::processSignal(frame);
};


