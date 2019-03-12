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
#include "veins/modules/mobility/traci/subscriptionManagement/TraCIVehicle.h"

namespace veins {
namespace TraCISubscriptionManagement {

TraCIVehicle::TraCIVehicle(double x, double y, std::string edgeID, double speed,
        double angle, std::string id, std::string typeID, int signals, double length,
        double height, double width)
    : TrafficParticipant(x, y, edgeID, speed, angle, id, typeID)
    , mSignals(signals)
    , mLength(length)
    , mHeight(height)
    , mWidth(width)
{
}

int TraCIVehicle::getSignals() {
    return mSignals;
}

double TraCIVehicle::getLength() {
    return mLength;
}

double TraCIVehicle::getHeight() {
    return mHeight;
}

double TraCIVehicle::getWidth() {
    return mWidth;
}

} // end namespace TraCISubscriptionManagement
} // end namespace Veins
