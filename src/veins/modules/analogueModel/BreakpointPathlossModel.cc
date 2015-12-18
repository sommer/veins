#include "veins/modules/analogueModel/BreakpointPathlossModel.h"

#include "veins/base/messages/AirFrame_m.h"

using Veins::AirFrame;

#define debugEV EV << "PhyLayer(BreakpointPathlossModel): "


void BreakpointPathlossModel::filterSignal(AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos) {
	Signal& signal = frame->getSignal();

	/** Calculate the distance factor */
	double distance = useTorus ? receiverPos.sqrTorusDist(sendersPos, playgroundSize)
								  : receiverPos.sqrdist(sendersPos);
	distance = sqrt(distance);
	debugEV << "distance is: " << distance << endl;

	if(distance <= 1.0) {
		//attenuation is negligible
		return;
	}

	double attenuation = 1;
	// PL(d) = PL0 + 10 alpha log10 (d/d0)
	// 10 ^ { PL(d)/10 } = 10 ^{PL0 + 10 alpha log10 (d/d0)}/10
	// 10 ^ { PL(d)/10 } = 10 ^ PL0/10 * 10 ^ { 10 log10 (d/d0)^alpha }/10
	// 10 ^ { PL(d)/10 } = 10 ^ PL0/10 * 10 ^ { log10 (d/d0)^alpha }
	// 10 ^ { PL(d)/10 } = 10 ^ PL0/10 * (d/d0)^alpha
	if(distance < breakpointDistance) {
		attenuation = attenuation * PL01_real;
		attenuation = attenuation * pow(distance, alpha1);
	} else {
		attenuation = attenuation * PL02_real;
		attenuation = attenuation * pow(distance/breakpointDistance, alpha2);
	}
	attenuation = 1/attenuation;
	debugEV << "attenuation is: " << attenuation << endl;

	if(debug) {
	  pathlosses.record(10*log10(attenuation)); // in dB
	}

	//const DimensionSet& domain = DimensionSet::timeDomain;
	Argument arg;	// default constructor initializes with a single dimension, time, and value 0 (offset from signal start)
	TimeMapping<Linear>* attMapping = new TimeMapping<Linear> ();	// mapping performs a linear interpolation from our single point -> constant
	attMapping->setValue(arg, attenuation);

	/* at last add the created attenuation mapping to the signal */
	signal.addAttenuation(attMapping);
}
