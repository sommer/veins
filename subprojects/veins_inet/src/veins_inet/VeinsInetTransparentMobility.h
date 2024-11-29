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

#pragma once

#include "inet/mobility/base/MobilityBase.h"
#if INET_VERSION >= 0x0404
#include "inet/common/ModuleRefByPar.h"
#endif

#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins_inet/veins_inet.h"

namespace veins {
using namespace omnetpp;

/**
 * This class aims to improve the interoperability between Veins based projects and INET.
 * In case that some module needs the default TraCIMobility from Veins, but the project should also use INET,
 * this mobility can be used to attach to the TraCIMobility and at the same time implements all functions INET expects to be present.
 * Not all values are returned correct, so this mobility module should be used with care, as current SUMO (1.21.0) only supports alpha for the orientation.
 */
class VEINS_INET_API VeinsInetTransparentMobility : public inet::MobilityBase, public cListener {
protected:
#if INET_VERSION >= 0x0404
    // The TraCIMobility module from Veins
    inet::ModuleRefByPar<TraCIMobility> mobility;
#else
    TracCIMobility mobility;
#endif
    inet::Coord positionOffset = inet::Coord::NIL;
    inet::Quaternion orientationOffset = inet::Quaternion::NIL;
    bool isZeroOffset = false;
    inet::Coord lastVelocity;
    inet::Quaternion lastAngularPosition;
    inet::Quaternion lastAngularVelocity = inet::Quaternion(0, 0, 0, 0);

protected:
    virtual int numInitStages() const override
    {
        return inet::NUM_INIT_STAGES;
    }
    virtual void initialize(int stage) override;
    virtual void handleSelfMessage(cMessage* msg) override
    {
        throw cRuntimeError("Unknown self message");
    }

public:
    virtual const inet::Coord& getCurrentPosition() override;
    virtual const inet::Coord& getCurrentVelocity() override;
    virtual const inet::Coord& getCurrentAcceleration() override;

    virtual const inet::Quaternion& getCurrentAngularPosition() override;
    virtual const inet::Quaternion& getCurrentAngularVelocity() override;
    virtual const inet::Quaternion& getCurrentAngularAcceleration() override;

    virtual void receiveSignal(cComponent* source, simsignal_t signal, cObject* object, cObject* details) override;
};

} // namespace veins
