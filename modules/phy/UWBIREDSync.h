/* -*- mode:c++ -*- ********************************************************
 * file:        UWBIREDSync.h
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
 * 				that synchronizes on the first sync preamble sequence that it
 * 				receives long enough and powerful enough.
 ***************************************************************************/

#ifndef UWBIREDSYNC_H_
#define UWBIREDSYNC_H_

#include <vector>
#include <map>
#include <math.h>

#include "Mapping.h"
#include "AirFrame_m.h"
#include "MacPkt_m.h"
#include "Decider.h"
#include "UWBIREnergyDetectionDeciderV2.h"

using namespace std;

class UWBIRPhyLayer;

class UWBIREDSync: public UWBIREnergyDetectionDeciderV2 {
public:
	UWBIREDSync(DeciderToPhyInterface* iface,
				UWBIRPhyLayer* _uwbiface, double _sensitivity,
				double _syncThreshold, bool _syncAlwaysSucceeds, bool _stats,
				bool _trace, double _tmin);
	~UWBIREDSync();

protected:
	virtual bool attemptSync(Signal* signal);
	bool evaluateEnergy(Signal* s);

private:
	simtime_t tmin;
	vector<AirFrame*> syncVector;
	Argument argSync;
};

#endif /* UWBIREDSYNC_H_ */
