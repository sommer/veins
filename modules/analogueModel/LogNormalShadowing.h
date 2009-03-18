//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef LOGNORMALSHADOWING_H_
#define LOGNORMALSHADOWING_H_

#include "AnalogueModel.h"

/**
 * @brief Channel state implementing log-normal shadowing.
 * @author Hermann S. Lichte, Karl Wessel (port for MiXiM)
 * @date 2007-08-15
 **/
class LogNormalShadowing: public AnalogueModel {
protected:
	/** @brief The dimensions of this analogue model*/
	static DimensionSet dimensions;

	/** @brief Mean of the random attenuation in dB */
	double mean;

	/** @brief Standart deviation of the random attenuation in dB */
	double stdDev;

	/** @brief The interval to set attenuation entries in. */
	simtime_t interval;

protected:
	/**
	 * @brief Returns a random log normal distributed gain factor.
	 *
	 * The gain factor is below 1.0 so its an actual attenuation.
	 */
	double randomLogNormalGain() const;

public:
	LogNormalShadowing(double mean, double stdDev, simtime_t interval);
	virtual ~LogNormalShadowing();

	/**
	 * @brief Calculates shadowing loss based on a normal gaussian function.
	 */
	virtual void filterSignal(Signal& s);
};

#endif /* LOGNORMALSHADOWING_H_ */
