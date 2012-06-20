//
// Copyright (C) 2006-2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef SIMPLEOBSTACLEFADING_H_
#define SIMPLEOBSTACLEFADING_H_

#include "AnalogueModel.h"
#include "Mapping.h"
#include "BaseWorldUtility.h"
#include "ObstacleControl.h"
#include <Move.h>
#include <Signal_.h>
#include <AirFrame_m.h>

#include <cstdlib>

/**
 * @brief
 * Uses an ObstacleControl module to calculate attenuation by obstacles.
 *
 * @ingroup analogueModels
 *
 * @author Christoph Sommer
 *
 * @see Obstacle
 * @see ObstacleControl
 */
class SimpleObstacleShadowing : public AnalogueModel
{
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
	virtual void filterSignal(AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos);
};

#endif
