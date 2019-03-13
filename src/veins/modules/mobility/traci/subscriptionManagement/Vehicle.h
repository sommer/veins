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

#include "veins/veins.h"

#include "veins/modules/mobility/traci/subscriptionManagement/TrafficParticipant.h"

namespace veins {
namespace TraCISubscriptionManagement {

/**
 * @struct Vehicle
 *
 * Extends the TrafficParticipant definition by a couple of properties
 * of the vehicle like length, height and width.
 *
 * @author Nico Dassler <dassler@hm.edu>
 */
struct Vehicle : TrafficParticipant {
    /**
     * Stores the signals of this vehicle.
     */
    int signals;

    /**
     * Stores the length of this vehicle.
     */
    double length;

    /**
     * Stores the height of this vehicle.
     */
    double height;

    /**
     * Stores the width of this vehicle.
     */
    double width;
};

} // end namespace TraCISubscriptionManagement
} // namespace veins
