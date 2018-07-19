#include "veins/modules/analogueModel/SimpleObstacleShadowing.h"

using namespace Veins;

using Veins::AirFrame;

#define debugEV EV << "PhyLayer(SimpleObstacleShadowing): "

SimpleObstacleShadowing::SimpleObstacleShadowing(ObstacleControl& obstacleControl, double carrierFrequency, bool useTorus, const Coord& playgroundSize, bool debug)
    : obstacleControl(obstacleControl)
    , carrierFrequency(carrierFrequency)
    , useTorus(useTorus)
    , playgroundSize(playgroundSize)
    , debug(debug)
{
    if (useTorus) throw cRuntimeError("SimpleObstacleShadowing does not work on torus-shaped playgrounds");
}

void SimpleObstacleShadowing::filterSignal(Signal* signal, const Coord& sendersPos, const Coord& receiverPos)
{

    double factor = obstacleControl.calculateAttenuation(sendersPos, receiverPos);

    debugEV << "value is: " << factor << endl;

    signal->addUniformAttenuation(factor);
}
