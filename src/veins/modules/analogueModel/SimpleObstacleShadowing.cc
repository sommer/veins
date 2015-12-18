#include "veins/modules/analogueModel/SimpleObstacleShadowing.h"

using Veins::AirFrame;

#define debugEV EV << "PhyLayer(SimpleObstacleShadowing): "

#if 0
SimplePathlossConstMapping::SimplePathlossConstMapping(const DimensionSet& dimensions,
													   SimpleObstacleShadowing* model,
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
		assert(pos.hasArgVal(Dimension::frequency()));
		freq = pos.getArgValue(Dimension::frequency());
	}
	double wavelength = BaseWorldUtility::speedOfLight() / freq;
	return (wavelength * wavelength) * distFactor;
}
#endif


SimpleObstacleShadowing::SimpleObstacleShadowing(ObstacleControl& obstacleControl, double carrierFrequency, bool useTorus, const Coord& playgroundSize, bool debug) :
	obstacleControl(obstacleControl),
	carrierFrequency(carrierFrequency),
	useTorus(useTorus),
	playgroundSize(playgroundSize),
	debug(debug)
{
	if (useTorus) throw cRuntimeError("SimpleObstacleShadowing does not work on torus-shaped playgrounds");
}


void SimpleObstacleShadowing::filterSignal(AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos) {
	Signal& s = frame->getSignal();

	double factor = obstacleControl.calculateAttenuation(sendersPos, receiverPos);

	debugEV << "value is: " << factor << endl;

	bool hasFrequency = s.getTransmissionPower()->getDimensionSet().hasDimension(Dimension::frequency());
	const DimensionSet& domain = hasFrequency ? DimensionSet::timeFreqDomain() : DimensionSet::timeDomain();
	ConstantSimpleConstMapping* attMapping = new ConstantSimpleConstMapping(domain, factor);
	s.addAttenuation(attMapping);
}
