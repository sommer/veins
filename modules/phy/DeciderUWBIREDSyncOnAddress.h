/* -*- mode:c++ -*- ********************************************************
 * file:        DeciderUWBIREDSyncOnAddress.h
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

#ifndef UWBIREDSYNCONADDRESS_H_
#define UWBIREDSYNCONADDRESS_H_

#include <vector>
#include <map>
#include <math.h>

#include "Mapping.h"
#include "AirFrame_m.h"
#include "MacPkt_m.h"
#include "Decider.h"
#include "DeciderUWBIRED.h"

using namespace std;

class PhyLayerUWBIR;

/**
 * @brief  this Decider models a non-coherent energy-detection receiver
 *
 * that that always synchronizes successfully on frames coming from
 * a particular node. It can be used to approximate quasi-orthogonal UWB
 * modulations and private time hopping sequences (you should also configure
 * the Decider adequately). It can also be used to study the robustness to
 * interference of the data segment while neglecting interference on the
 * sync preamble.
 *
 * @ingroup ieee802154a
 * @ingroup decider
*/

class DeciderUWBIREDSyncOnAddress: public DeciderUWBIRED {

public:
	DeciderUWBIREDSyncOnAddress(DeciderToPhyInterface* iface,
				PhyLayerUWBIR* _uwbiface,
				double _syncThreshold, bool _syncAlwaysSucceeds, bool _stats,
				bool _trace, int _addr, bool alwaysFailOnDataInterference);

	virtual bool attemptSync(Signal* signal);

	virtual simtime_t processSignal(AirFrame* frame);

protected:
	AirFrame* currFrame;
	int syncAddress;
};

#endif /* UWBIREDSYNCONADDRESS_H_ */
