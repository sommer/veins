#include "BreakpointPathlossModel.h"

#define debugEV (ev.isDisabled()||!debug) ? ev : ev << "PhyLayer(BreakpointPathlossModel): "

//void BreakpointPathlossModel::filterSignal(Signal& s){
//
//	/** Get start of the signal */
//	simtime_t sStart = s.getSignalStart();
//	simtime_t sEnd = s.getSignalLength() + sStart;
//
//	/** claim the Move pattern of the sender from the Signal */
//	Coord sendersPos = s.getMove().getPositionAt(sStart);
//	Coord myPos = myMove.getPositionAt(sStart);
//
//	/** Calculate the distance factor */
//	double sqrDistance = useTorus ? myPos.sqrTorusDist(sendersPos, playgroundSize)
//								  : myPos.sqrdist(sendersPos);
//
//	debugEV << "sqrdistance is: " << sqrDistance << endl;
//
//	if(sqrDistance <= breakpointDistance2) {
//		closeRangeModel.filterSignal(s);
//	} else {
//		farRangeModel.filterSignal(s);
//	}
//}

void BreakpointPathlossModel::filterSignal(Signal& s) {

	/** Get start of the signal */
	simtime_t sStart = s.getSignalStart();
	simtime_t sEnd = s.getSignalLength() + sStart;

	/** claim the Move pattern of the sender from the Signal */
	Coord sendersPos = s.getMove().getPositionAt(sStart);
	Coord myPos = myMove.getPositionAt(sStart);

	/** Calculate the distance factor */
	double distance = useTorus ? myPos.sqrTorusDist(sendersPos, playgroundSize)
								  : myPos.sqrdist(sendersPos);
	distance = sqrt(distance);
	debugEV << "distance is: " << distance << endl;

	if(distance <= 1.0) {
		//attenuation is negligible
		return;
	}

	double attenuation = 1;
	if(distance < breakpointDistance) {
		attenuation = attenuation * PL01_real;
		attenuation = attenuation * pow(distance, -alpha1) / (4*M_PI);
	} else {
		attenuation = attenuation * PL02_real;
		attenuation = attenuation * pow(distance, -alpha2) / (4*M_PI);
	}
	double wavelength = BaseWorldUtility::speedOfLight / carrierFrequency;
	double aperture = (wavelength * wavelength) / (4*M_PI);
	attenuation = aperture * attenuation;
	debugEV << "attenuation is: " << attenuation << endl;

	pathlosses.record(10*log10(attenuation)); // in dB

	const DimensionSet& domain = DimensionSet::timeDomain;
	Argument arg;	// default constructor initializes with a single dimension, time, and value 0 (offset from signal start)
	TimeMapping<Linear>* attMapping = new TimeMapping<Linear> ();	// mapping performs a linear interpolation from our single point -> constant
	attMapping->setValue(arg, attenuation);

	/* at last add the created attenuation mapping to the signal */
	s.addAttenuation(attMapping);
}
