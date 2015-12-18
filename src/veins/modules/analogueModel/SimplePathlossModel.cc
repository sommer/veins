#include "veins/modules/analogueModel/SimplePathlossModel.h"

#include "veins/base/messages/AirFrame_m.h"

using Veins::AirFrame;

#define splmEV EV << "PhyLayer(SimplePathlossModel): "

SimplePathlossConstMapping::SimplePathlossConstMapping(const DimensionSet& dimensions,
													   SimplePathlossModel* model,
													   const double distFactor) :
	SimpleConstMapping(dimensions),
	distFactor(distFactor),
	model(model),
	hasFrequency(dimensions.hasDimension(Dimension::frequency()))
{
}

double SimplePathlossConstMapping::getValue(const Argument& pos) const
{
	double freq = model->carrierFrequency;
	if(hasFrequency) {
		assert(pos.hasArgVal(Dimension::frequency()));
		freq = pos.getArgValue(Dimension::frequency());
	}
	double wavelength = BaseWorldUtility::speedOfLight() / freq;
	return (wavelength * wavelength) * distFactor;
}



void SimplePathlossModel::filterSignal(AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos)
{
	Signal& signal = frame->getSignal();

	/** Calculate the distance factor */
	double sqrDistance = useTorus ? receiverPos.sqrTorusDist(sendersPos, playgroundSize)
								  : receiverPos.sqrdist(sendersPos);

	splmEV << "sqrdistance is: " << sqrDistance << endl;

	if(sqrDistance <= 1.0) {
		//attenuation is negligible
		return;
	}

	// wavelength in meters (this is only used for debug purposes here
	// the actual effect of the wavelength on the attenuation is
	// calculated in SimplePathlossConstMappings "getValue()" method).
	double wavelength = (BaseWorldUtility::speedOfLight()/carrierFrequency);
	splmEV << "wavelength is: " << wavelength << endl;

	// the part of the attenuation only depending on the distance
	double distFactor = pow(sqrDistance, -pathLossAlphaHalf) / (16.0 * M_PI * M_PI);
	splmEV << "distance factor is: " << distFactor << endl;

	//is our signal to attenuate defined over frequency?
	bool hasFrequency = signal.getTransmissionPower()->getDimensionSet().hasDimension(Dimension::frequency());
	splmEV << "Signal contains frequency dimension: " << (hasFrequency ? "yes" : "no") << endl;

	const DimensionSet& domain = hasFrequency ? DimensionSet::timeFreqDomain() : DimensionSet::timeDomain();

	//create the Attenuation mapping which takes the distance factor as parameter
	//to calculate the attenuation from this and the frequency used for the transmission
	//see the classes "getValue()" for more
	SimplePathlossConstMapping* attMapping = new SimplePathlossConstMapping(
													domain,
													this,
													distFactor);

	/* at last add the created attenuation mapping to the signal */
	signal.addAttenuation(attMapping);
}

double SimplePathlossModel::calcPathloss(const Coord& receiverPos, const Coord& sendersPos)
{
	/*
	 * maybe we can reuse an already calculated value for the square-distance
	 * at this point.
	 *
	 */
	double sqrdistance = 0.0;

	if (useTorus)
	{
		sqrdistance = receiverPos.sqrTorusDist(sendersPos, playgroundSize);
	} else
	{
		sqrdistance = receiverPos.sqrdist(sendersPos);
	}

	splmEV << "sqrdistance is: " << sqrdistance << endl;

	double attenuation = 1.0;
	// wavelength in metres
	double wavelength = (BaseWorldUtility::speedOfLight()/carrierFrequency);

	splmEV << "wavelength is: " << wavelength << endl;

	if (sqrdistance > 1.0)
	{
		attenuation = (wavelength * wavelength) / (16.0 * M_PI * M_PI)
						* (pow(sqrdistance, -1.0*pathLossAlphaHalf));
	}

	splmEV << "attenuation is: " << attenuation << endl;

	return attenuation;
}
