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

#ifndef JAKESFADING_H_
#define JAKESFADING_H_

#include "AnalogueModel.h"

class JakesFading;

/**
 * @brief Mapping used to represent attenuation of a signal by JakesFading.
 *
 * @ingroup analogueModels
 * @ingroup mapping
 */
class JakesFadingMapping: public SimpleConstMapping {
protected:
	static DimensionSet dimensions;

	/** @brief Pointer to the model.*/
	JakesFading* model;

	/** @brief The relative speed between the two hosts for this attenuation.*/
	double relSpeed;

public:
	/**
	 * @brief Takes the model, the relative speed between two hosts and
	 * the interval in which to create key entries.
	 */
	JakesFadingMapping(JakesFading* model, double relSpeed,
					   const Argument& start,
					   const Argument& interval,
					   const Argument& end):
		SimpleConstMapping(dimensions, start, end, interval),
		model(model), relSpeed(relSpeed)
	{}

	virtual double getValue(const Argument& pos) const;

	/**
	 * @brief creates a clone of this mapping.
	 *
	 * This method has to be implemented by every subclass.
	 * But most time the implementation will look like the
	 * implementation of this method (except of the class name).
	 */
	ConstMapping* constClone() const
	{
		return new JakesFadingMapping(*this);
	}
};

/**
 * @brief Implements Rayleigh fading after Jakes' model.
 *
 * @ingroup analogueModels
 * @author Hermann S. Lichte, Karl Wessel (port for MiXiM)
 */
class JakesFading: public AnalogueModel {
protected:
	friend class JakesFadingMapping;

	/** @brief Number of fading paths used. */
	int fadingPaths;

	/**
	 * @brief Angle of arrival on a fading path used for Doppler shift calculation.
	 **/
	double* angleOfArrival;

	/** @brief Delay on a fading path. */
	simtime_t* delay;

	/** @brief HostMove of this analogue models host. */
	Move* hostMove;

	/** @brief Carrier frequency to be used. */
	double carrierFrequency;

	/** @brief The interval to set attenuation entries in. */
	Argument interval;

public:
	/**
	 * @brief Takes the number of fading paths, the maximum delay
	 * on a path, the hosts move, the carrier frequency used and the
	 * interval in which to defien attenuation entries in.	 *
	 */
	JakesFading(int fadingPaths, simtime_t delayRMS,
				Move* hostMove, double carrierFrequency,
				simtime_t interval);
	virtual ~JakesFading();

	virtual void filterSignal(Signal& s);
};

#endif /* JAKESFADING_H_ */
