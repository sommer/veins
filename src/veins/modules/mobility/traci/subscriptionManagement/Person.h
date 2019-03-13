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
 * @struct Person
 *
 * Person is nothing more than a simple TrafficParticipant
 * but can be extended in the future.
 *
 * @author Nico Dassler <dassler@hm.edu>
 */
struct Person: public TrafficParticipant {
};

} // end namespace TraCISubscriptionManagement
} // namespace veins
