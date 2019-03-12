//
// Copyright (C) 2006-2017 Christoph Sommer <sommer@ccs-labs.org>
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

#ifndef SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_TRACITRAFFICLIGHT_H_
#define SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_TRACITRAFFICLIGHT_H_

#include <string>

#include "veins/veins.h"

namespace veins {
namespace TraCISubscriptionManagement {

/**
 * @struct TraCITrafficLight
 *
 * Simple definition of a traffic light as data
 * exchange format.
 *
 * @author Nico Dassler <dassler@hm.edu>
 */
struct TraCITrafficLight {
    /**
     * The id of the traffic light.
     */
    std::string id;

    /**
     * The index of the current phase in the current program.
     */
    int32_t currentPhase;

    /**
     * The id of the current program.
     */
    std::string currentProgram;

    /**
     * The assumed time (in seconds) at which the tls changes
     * the phase. Please note that the time to switch is not
     * relative to current simulation step (the result returned
     * by the query will be absolute time, counting from
     * simulation start); to obtain relative time, one
     *  needs to subtract current simulation time from the
     *  result returned by this query. Please also note that
     *  the time may vary in the case of actuated/adaptive
     *  traffic lights.
     *  (from: https://sumo.dlr.de/wiki/TraCI/Traffic_Lights_Value_Retrieval)
     */
    simtime_t nextSwitch;

    /**
     * The named tl's state as a tuple of light definitions
     * from rRgGyYoO, for red, green, yellow, off, where
     * lower case letters mean that the stream has to decelerate.
     * (from: https://sumo.dlr.de/wiki/TraCI/Traffic_Lights_Value_Retrieval)
     */
    std::string redYellowGreenState;
};

} // end namespace TraCISubscriptionManagement
} // end namespace Veins

#endif /* SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_TRACITRAFFICLIGHT_H_ */
