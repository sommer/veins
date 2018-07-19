#ifndef SIMPLEOBSTACLEFADING_H_
#define SIMPLEOBSTACLEFADING_H_

#include "veins/base/phyLayer/AnalogueModel.h"
#include "veins/base/modules/BaseWorldUtility.h"
#include "veins/modules/obstacle/ObstacleControl.h"
#include "veins/base/utils/Move.h"
#include "veins/base/toolbox/Signal.h"
#include "veins/base/messages/AirFrame_m.h"

using Veins::AirFrame;
using Veins::ObstacleControl;

#include <cstdlib>

namespace Veins {

/**
 * @brief Basic implementation of a SimpleObstacleShadowing
 *
 * @ingroup analogueModels
 */
class SimpleObstacleShadowing : public AnalogueModel {
protected:
    /** @brief reference to global ObstacleControl instance */
    ObstacleControl& obstacleControl;

    /** @brief carrier frequency needed for calculation */
    double carrierFrequency;

    /** @brief Information needed about the playground */
    const bool useTorus;

    /** @brief The size of the playground.*/
    const Coord& playgroundSize;

    /** @brief Whether debug messages should be displayed. */
    bool debug;

public:
    /**
     * @brief Initializes the analogue model. myMove and playgroundSize
     * need to be valid as long as this instance exists.
     *
     * The constructor needs some specific knowledge in order to create
     * its mapping properly:
     *
     * @param carrierFrequency the carrier frequency
     * @param myMove a pointer to the hosts move pattern
     * @param useTorus information about the playground the host is moving in
     * @param playgroundSize information about the playground the host is moving in
     * @param debug display debug messages?
     */
    SimpleObstacleShadowing(ObstacleControl& obstacleControl, double carrierFrequency, bool useTorus, const Coord& playgroundSize, bool debug);

    /**
     * @brief Filters a specified Signal by adding an attenuation
     * over time to the Signal.
     */
    virtual void filterSignal(Signal* signal, const Coord& sendersPos, const Coord& receiverPos);
};

} // namespace Veins

#endif /*PATHLOSSMODEL_H_*/
