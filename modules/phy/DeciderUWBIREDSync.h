/*
 * DeciderUWBIREDSync.h
 * Author: Jerome Rousselot <jerome.rousselot@csem.ch>
 * Copyright: (C) 2008-2010 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 *              Wireless Embedded Systems
 *              Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 */

#ifndef UWBIREDSYNC_H_
#define UWBIREDSYNC_H_

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
 * that synchronizes on the first sync preamble sequence
 * that is "long enough" and "powerful enough". This is defined with
 * the respective fields tmin and syncThreshold.
 *
 * @ingroup ieee802154a
 * @ingroup decider
*/
class DeciderUWBIREDSync: public DeciderUWBIRED {
public:
	DeciderUWBIREDSync(DeciderToPhyInterface* iface,
				PhyLayerUWBIR* _uwbiface,
				double _syncThreshold, bool _syncAlwaysSucceeds, bool _stats,
				bool _trace, double _tmin, bool alwaysFailOnDataInterference);
	~DeciderUWBIREDSync();

protected:
	virtual bool attemptSync(Signal* signal);
	bool evaluateEnergy(Signal* s);

private:
	simtime_t tmin;
	vector<AirFrame*> syncVector;
	Argument argSync;
};

#endif /* UWBIREDSYNC_H_ */
