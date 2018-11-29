//
// Copyright (C) 2006-2018 Christoph Sommer <sommer@ccs-labs.org>
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

#include "veins/modules/analogueModel/VehicleObstacleShadowing.h"

using namespace Veins;

VehicleObstacleShadowing::VehicleObstacleShadowing(VehicleObstacleControl& vehicleObstacleControl, double carrierFrequency, bool useTorus, const Coord& playgroundSize)
    : vehicleObstacleControl(vehicleObstacleControl)
    , carrierFrequency(carrierFrequency)
    , useTorus(useTorus)
    , playgroundSize(playgroundSize)
{
    vehicleObstacleControl.setCarrierFrequency(carrierFrequency);
    if (useTorus) throw cRuntimeError("VehicleObstacleShadowing does not work on torus-shaped playgrounds");
}

void VehicleObstacleShadowing::filterSignal(Signal* signal, const Coord& senderPos, const Coord& receiverPos)
{
    auto potentialObstacles = vehicleObstacleControl.getPotentialObstacles(senderPos, receiverPos, *signal);

    if (potentialObstacles.size() < 1) return;

    double senderHeight = senderPos.z;
    double receiverHeight = receiverPos.z;
    potentialObstacles.insert(potentialObstacles.begin(), std::make_pair(0, senderHeight));
    potentialObstacles.emplace_back(senderPos.distance(receiverPos), receiverHeight);

    double attenuationDB = VehicleObstacleControl::getVehicleAttenuationDZ(potentialObstacles, carrierFrequency);

    EV << "t=" << simTime() << ": Attenuation by vehicles is " << attenuationDB << " dB" << std::endl;

    auto factor = pow(10.0, -attenuationDB / 10.0);

    EV_TRACE << "value is: " << factor << endl;

    signal->addUniformAttenuation(factor);
}
