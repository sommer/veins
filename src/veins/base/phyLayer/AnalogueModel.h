#pragma once

#include <memory>
#include <vector>

#include "veins/veins.h"

#include "veins/base/utils/AntennaPosition.h"
#include "veins/base/utils/Coord.h"
#include "veins/modules/utility/HasLogProxy.h"

namespace Veins {

class AirFrame;
class Signal;

/**
 * @brief Interface for the analogue models of the physical layer.
 *
 * An analogue model is a filter responsible for changing
 * the attenuation value of a Signal to simulate things like
 * shadowing, fading, pathloss or obstacles.
 *
 * @ingroup analogueModels
 */
class VEINS_API AnalogueModel : public HasLogProxy {

public:
    AnalogueModel(cComponent* owner)
        : HasLogProxy(owner)
    {
    }

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
     */
    virtual void filterSignal(Signal* signal) = 0;

    /**
     * If the model never increases the power level of any signal given to filterSignal, it returns true here.
     * This allows optimized signal handling.
     */
    virtual bool neverIncreasesPower()
    {
        return false;
    }
};

using AnalogueModelList = std::vector<std::unique_ptr<AnalogueModel>>;

} // namespace Veins
