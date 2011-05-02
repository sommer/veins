#ifndef PER_MODEL_H
#define PER_MODEL_H

#include "AnalogueModel.h"

class PERModel : public AnalogueModel {
protected:
	double packetErrorRate;
public:
	/** @brief The PERModel constructor takes as argument the packet error rate to apply (must be between 0 and 1). */
	PERModel(double per): packetErrorRate(per) { assert(per <= 1 && per >= 0);}

	virtual void filterSignal(Signal& s, bool isActiveAtOrigin);

	virtual bool isActiveAtDestination() { return true; }

	virtual bool isActiveAtOrigin() { return false; }

	virtual void setDestinationChannelAccess(ChannelAccess* ca) { ;	}

};

#endif
