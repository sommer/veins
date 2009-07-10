#include "SimplePathlossModel.h"

#define debugEV (ev.isDisabled()||!debug) ? ev : ev << "PhyLayer(SimplePathlossModel): "

SimplePathlossConstMapping::SimplePathlossConstMapping(const DimensionSet& dimensions,
													   SimplePathlossModel* model,
													   const double distFactor) :
	SimpleConstMapping(dimensions),
	distFactor(distFactor),
	model(model),
	hasFrequency(dimensions.hasDimension(Dimension::frequency_static()))
{
}

double SimplePathlossConstMapping::getValue(const Argument& pos) const
{
	double freq = model->carrierFrequency;
	if(hasFrequency) {
		assert(pos.hasArgVal(Dimension::frequency));
		freq = pos.getArgValue(Dimension::frequency);
	}
	double wavelength = BaseWorldUtility::speedOfLight / freq;
	return (wavelength * wavelength) * distFactor;
}



void SimplePathlossModel::filterSignal(Signal& s){

	/** Get start of the signal */
	simtime_t sStart = s.getSignalStart();
	simtime_t sEnd = s.getSignalLength() + sStart;

	/** claim the Move pattern of the sender from the Signal */
	Coord sendersPos = s.getMove().getPositionAt(sStart);
	Coord myPos = myMove.getPositionAt(sStart);

	/** Calculate the distance factor */
	double sqrDistance = useTorus ? myPos.sqrTorusDist(sendersPos, playgroundSize)
								  : myPos.sqrdist(sendersPos);

	debugEV << "sqrdistance is: " << sqrDistance << endl;

	if(sqrDistance <= 1.0) {
		//attenuation is negligible
		return;
	}

	// wavelength in metres
	double wavelength = (BaseWorldUtility::speedOfLight/carrierFrequency);
	debugEV << "wavelength is: " << wavelength << endl;

	double distFactor = pow(sqrDistance, -pathLossAlphaHalf) / (16.0 * M_PI * M_PI);
	debugEV << "distance factor is: " << distFactor << endl;

	bool hasFrequency = s.getTransmissionPower()->getDimensionSet().hasDimension(Dimension::frequency);

	const DimensionSet& domain = hasFrequency ? DimensionSet::timeFreqDomain : DimensionSet::timeDomain;

	SimplePathlossConstMapping* attMapping = new SimplePathlossConstMapping(
													domain,
													this,
													distFactor);

	/* at last add the created attenuation mapping to the signal */
	s.addAttenuation(attMapping);
}

double SimplePathlossModel::calcPathloss(const Coord& myPos, const Coord& sendersPos)
{
	/*
	 * maybe we can reuse an already calculated value for the square-distance
	 * at this point.
	 *
	 */
	double sqrdistance = 0.0;

	if (useTorus)
	{
		sqrdistance = myPos.sqrTorusDist(sendersPos, playgroundSize);
	} else
	{
		sqrdistance = myPos.sqrdist(sendersPos);
	}

	debugEV << "sqrdistance is: " << sqrdistance << endl;

	double attenuation = 1.0;
	// wavelength in metres
	double wavelength = (BaseWorldUtility::speedOfLight/carrierFrequency);

	debugEV << "wavelength is: " << wavelength << endl;

	if (sqrdistance > 1.0)
	{
		attenuation = (wavelength * wavelength) / (16.0 * M_PI * M_PI)
						* (pow(sqrdistance, -1.0*pathLossAlphaHalf));
	}

	debugEV << "attenuation is: " << attenuation << endl;

	return attenuation;
}
