#pragma once

#include "veins/veins.h"

#include "veins/base/utils/Coord.h"
#include "veins/base/toolbox/Signal.h"

namespace Veins {

class AirFrame;

using Veins::AirFrame;

/**
 * @brief Interface for the analogue models of the physical layer.
 *
 * An analogue model is a filter responsible for changing
 * the attenuation value of a Signal to simulate things like
 * shadowing, fading, pathloss or obstacles.
 *
 * @ingroup analogueModels
 */
class VEINS_API AnalogueModel {

public:
    virtual ~AnalogueModel()
    {
    }

    /**
     * @brief Has to be overriden by every implementation.
     *
     * Filters a specified AirFrame's Signal by adding an attenuation
     * over time to the Signal.
     *
     * @param signal        The signal to filter.
     * @param senderPos    The position of the frame sender.
     * @param receiverPos    The position of frame receiver.
     */
    virtual void filterSignal(Signal* signal, const Coord& senderPos, const Coord& receiverPos) = 0;

    /**
     * If the model never increases the power level of any signal given to filterSignal, it returns true here.
     * This allows optimized signal handling.
     */
    virtual bool neverIncreasesPower()
    {
        return false;
    }
};

} // namespace Veins
