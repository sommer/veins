#ifndef ANALOGUEMODEL_
#define ANALOGUEMODEL_

#include "Signal_.h"

/**
 * Interface for the analogue models of the physical layer.
 * 
 * An analogue model is a filter responsible for changing
 * the attenuation value of a Signal to simulate things like
 * shadowing, fading, pathloss or obstacles.
 */
class AnalogueModel {
	
public:
	/**
	 * @brief Has to be overriden by every implementation.
	 * 
	 * Filters a specified Signal by adding an attenuation
	 * over time to the Signal.
	 * 
	 * Returns a reference to the changed Signal. Normally 
	 * this would be the same reference as the one given 
	 * to the function previously. 
	 */
	virtual Signal& filterSignal(Signal& s) = 0;
};

#endif /*ANALOGUEMODEL_*/
