//
// Copyright (C) 2024 Jannusch Bigge <jannusch.bigge@tu-dresden.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
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

#include "veins_inet/VeinsInetTransparentMobility.h"

#include "inet/common/InitStages.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/EulerAngles.h"
#include "inet/mobility/base/MobilityBase.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/common/Units.h"

#include "veins/base/modules/BaseMobility.h"
#include "veins/base/utils/Coord.h"

namespace veins {

Define_Module(VeinsInetTransparentMobility);

#if INET_VERSION >= 0x0404
void VeinsInetTransparentMobility::initialize(int stage)
{
    // set position so that the mobility base does not complain
    lastPosition = inet::Coord::ZERO;
    inet::MobilityBase::initialize(stage);
    if (stage == inet::INITSTAGE_LOCAL) {
        mobility.reference(this, "mobilityModule", true);
        positionOffset.x = par("offsetX");
        positionOffset.y = par("offsetY");
        positionOffset.z = par("offsetZ");

        auto alpha = inet::deg(par("offsetHeading"));
        auto offsetElevation = inet::deg(par("offsetElevation"));
        auto beta = -offsetElevation;
        auto gamma = inet::deg(par("offsetBank"));
        orientationOffset = inet::Quaternion(inet::EulerAngles(alpha, beta, gamma));
        isZeroOffset = positionOffset == inet::Coord::ZERO;
        check_and_cast<cModule*>(mobility.get())->subscribe(BaseMobility::mobilityStateChangedSignal, this);
        WATCH(lastVelocity);
        WATCH(lastAngularPosition);
    }
    ;
    // We patch the OMNeT++ Display String to set the initial position. Make sure this works.
    ASSERT(hasPar("initFromDisplayString") && par("initFromDisplayString"));
}
#else
void VeinsInetTransparentMobility::initialize(int stage)
{
    throw cRuntimeError("Module not supported with current INET version. Pls use at least INET 4.4.0")
}
#endif

void VeinsInetTransparentMobility::receiveSignal(cComponent* source, simsignal_t signal, cObject* object, cObject* details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (BaseMobility::mobilityStateChangedSignal == signal)
        emitMobilityStateChangedSignal();
}

const inet::Coord& VeinsInetTransparentMobility::getCurrentPosition()
{
    auto lastVeinsPosition = mobility->getPositionAt(simTime());
    lastPosition = inet::Coord(lastVeinsPosition.x + positionOffset.x, lastVeinsPosition.y + positionOffset.y, lastVeinsPosition.z = positionOffset.z);
    return lastPosition;
}

const inet::Coord& VeinsInetTransparentMobility::getCurrentVelocity()
{
    // ATTENTION: not tested! TODO: Test this
    auto speed = mobility->getSpeed();
    inet::Coord direction = inet::Quaternion(inet::EulerAngles(inet::rad(mobility->getHeading().getRad()), inet::rad(0.0), inet::rad(0.0))).rotate(inet::Coord::X_AXIS);
    lastVelocity = direction * speed;
    return lastVelocity;
}

const inet::Coord& VeinsInetTransparentMobility::getCurrentAcceleration()
{
    throw cRuntimeError("Invalid operation");
}

const inet::Quaternion& VeinsInetTransparentMobility::getCurrentAngularPosition()
{
    // sumo only support the alpha value, leading to beat and gamma being zero
    auto heading = mobility->getHeading();
    lastAngularPosition = inet::Quaternion(inet::EulerAngles(inet::rad(heading.getRad()), inet::rad(0.0), inet::rad(0.0)));
    lastAngularPosition *= inet::Quaternion(orientationOffset);
    return lastAngularPosition;
}

const inet::Quaternion& VeinsInetTransparentMobility::getCurrentAngularVelocity()
{
    return lastAngularVelocity;
}

const inet::Quaternion& VeinsInetTransparentMobility::getCurrentAngularAcceleration()
{
    throw cRuntimeError("Invalid operation");
}
} // namespace veins
