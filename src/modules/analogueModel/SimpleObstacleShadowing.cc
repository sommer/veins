#include "SimpleObstacleShadowing.h"

#define debugEV (ev.isDisabled()||!debug) ? ev : ev << "PhyLayer(SimpleObstacleShadowing): "

SimpleObstacleShadowing::SimpleObstacleShadowing(ObstacleControl& obstacleControl, double carrierFrequency, bool useTorus, const Coord& playgroundSize, bool debug) :
	obstacleControl(obstacleControl),
	carrierFrequency(carrierFrequency),
	useTorus(useTorus),
	playgroundSize(playgroundSize),
	debug(debug)
{
	if (useTorus) opp_error("SimpleObstacleShadowing does not work on torus-shaped playgrounds");
}


void SimpleObstacleShadowing::filterSignal(AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos) {
	Signal& s = frame->getSignal();

	double factor = obstacleControl.calculateAttenuation(sendersPos, receiverPos);

	debugEV << "value is: " << factor << endl;

	bool hasFrequency = s.getTransmissionPower()->getDimensionSet().hasDimension(Dimension::frequency);
	const DimensionSet& domain = hasFrequency ? DimensionSet::timeFreqDomain : DimensionSet::timeDomain;
	ConstantSimpleConstMapping* attMapping = new ConstantSimpleConstMapping(domain, factor);
	s.addAttenuation(attMapping);
}
