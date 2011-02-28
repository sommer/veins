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
	double value = 0;
	if(model->L0 == -1) {
		// use analytical loss at 1 meter, apply frequency aperture dependency here
		double freq = model->carrierFrequency;
		if(hasFrequency) {
			assert(pos.hasArgVal(Dimension::frequency));
			freq = pos.getArgValue(Dimension::frequency);
		}
		double wavelength = BaseWorldUtility::speedOfLight / freq;
		value = (wavelength * wavelength) * distFactor;
	} else {
		// use measured loss at 1 meter, nothing to do
		value = distFactor;
	}
	return value;
}


void SimplePathlossModel::filterSignal(Signal& s, bool isActiveAtOrigin){

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

	// wavelength in meters (this is only used for debug purposes here
	// the actual effect of the wavelength on the attenuation is
	// calculated in SimplePathlossConstMappings "getValue()" method).
	double wavelength = (BaseWorldUtility::speedOfLight/carrierFrequency);
	debugEV << "wavelength is: " << wavelength << endl;

	// the part of the attenuation only depending on the distance
	double distFactor;
	double attenuation = 0;
	if (sqrDistance > 1.0)
	{
		// energy spread over a sphere of radius distance
		attenuation = pow(sqrDistance, -pathLossAlphaHalf) / (4.0 * M_PI);
		if(L0 == -1) {
			// theoretical L0 path loss (without Lambda², applied later in getValue)
			attenuation = attenuation / (4.0 * M_PI);
		} else {
			// use user-defined (measured / estimated) L0
			attenuation = attenuation * L0;
		}
	}
	debugEV << "distance factor is: " << attenuation << endl;

	//is our signal to attenuate defined over frequency?
	bool hasFrequency = s.getTransmissionPower()->getDimensionSet().hasDimension(Dimension::frequency);
	debugEV << "Signal contains frequency dimension: " << (hasFrequency ? "yes" : "no") << endl;

	const DimensionSet& domain = hasFrequency ? DimensionSet::timeFreqDomain : DimensionSet::timeDomain;

	//create the Attenuation mapping which takes the distance factor as parameter
	//to calculate the attenuation from this and the frequency used for the transmission
	//see the classes "getValue()" for more
	SimplePathlossConstMapping* attMapping = new SimplePathlossConstMapping(
													domain,
													this,
													attenuation);

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
