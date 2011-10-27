/* -*- mode:c++ -*- ********************************************************
 * Energy Framework for Omnet++, version 0.9
 *
 * Author:  Laura Marie Feeney
 *
 * Copyright 2009 Swedish Institute of Computer Science.
 *
 * This software is provided `as is' and without any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.
 *
 ***************************************************************************/

#ifndef BATTERYSTATE_H
#define BATTERYSTATE_H

#include <sstream>
#include <string>
#include <omnetpp.h>

#include "MiXiMDefs.h"

/**
 * @brief residual capacity of battery
 *
 * BatteryState is passed to BatteryStats and should not be used by
 * non-statistics modules.  Value may be read as absolute or relative
 * (to nominal) capacity.
 *
 * @ingroup power
 */
class MIXIM_API BatteryState : public cObject
{
protected:
  /** @brief nominal battery capacity in mW-s (mA-s at nominal voltage) */
  double nominal;
  /** @brief remaining battery capacity in mW-s */
  double absolute;

public:

    /**
     * @brief Initializes this battery state with the passed nominal
     * capacity.
     */
	BatteryState(double n=-1) :
		cObject(), nominal(n)
	{};

	/** @brief Residual capacity of battery (relative to nominal capacity). */
	double getRel() const {
		return absolute/nominal;
	}

	/** @brief Residual capacity of battery (in mW-s). */
	double getAbs() const {
		return absolute;
	}

	/** @brief Sets the residual capacity of battery (in mW-s).*/
	void set(double s) {
		absolute = s;
	}

	/** @brief Returns a printable string of this battery state.*/
	std::string info() const {
		std::ostringstream ost;
		ost << " battery at "<<  (absolute/nominal) * 100 <<"%";
		return ost.str();
	}
};

#endif
