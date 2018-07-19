#ifndef ANALOGUEMODEL_
#define ANALOGUEMODEL_

#include "veins/base/utils/MiXiMDefs.h"
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
class MIXIM_API AnalogueModel {

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
     * @param sendersPos    The position of the frame sender.
     * @param receiverPos    The position of frame receiver.
     */
    virtual void filterSignal(Signal* signal, const Coord& sendersPos, const Coord& receiverPos) = 0;
};

} // namespace Veins

#endif /*ANALOGUEMODEL_*/
