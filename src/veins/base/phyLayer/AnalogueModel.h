#ifndef ANALOGUEMODEL_
#define ANALOGUEMODEL_

#include "veins/base/utils/MiXiMDefs.h"
#include "veins/base/utils/Coord.h"

namespace Veins {
class AirFrame;
}
using Veins::AirFrame;

/**
 * @brief Interface for the analogue models of the physical layer.
 *
 * An analogue model is a filter responsible for changing
 * the attenuation value of a Signal to simulate things like
 * shadowing, fading, pathloss or obstacles.
 *
 * Note: The Mapping this an AnalogeuModel adds to a signal has
 * to define absolute time positions not relative.
 * Meaning the position zero refers to the simulation start not
 * the signal start.
 *
 * @ingroup analogueModels
 */
class MIXIM_API AnalogueModel {

public:
	virtual ~AnalogueModel() {}

	/**
	 * @brief Has to be overriden by every implementation.
	 *
	 * Filters a specified AirFrame's Signal by adding an attenuation
	 * over time to the Signal.
	 *
	 * @param frame			The incomming frame.
	 * @param sendersPos	The position of the frame sender.
	 * @param receiverPos	The position of frame receiver.
	 */
	virtual void filterSignal(AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos) = 0;
};

#endif /*ANALOGUEMODEL_*/
