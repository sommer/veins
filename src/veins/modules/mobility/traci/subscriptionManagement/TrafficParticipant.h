//
// Copyright (C) 2006-2017 Nico Dassler <dassler@hm.edu>
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

#pragma once

#include <string>

#include "veins/veins.h"

namespace veins {
namespace TraCISubscriptionManagement {

/**
 * @struct TrafficParticipant
 *
 * Provides a basic interface to variables that each traffic
 * participant shares. Extend this to add variables
 * for specific participants like cars, bikes, pedestrians etc.
 *
 * @author Nico Dassler <dassler@hm.edu>
 */
struct TrafficParticipant {

    /**
     * Stores the x coordinate of this participant.
     */
    double x;

    /**
     * Stores the y coordinate of this participant.
     */
    double y;

    /**
     * Stores the edge id of this participant.
     */
    std::string edgeID;

    /**
     * Stores the speed of this participant.
     */
    double speed;

    /**
     * Stores the angle of this participant.
     */
    double angle;

    /**
     * Stores the id of this person.
     */
    std::string id;

    /**
     * Stores the type id of this person.
     */
    std::string typeID;
};

} // end namespace TraCISubscriptionManagement
} // namespace veins
