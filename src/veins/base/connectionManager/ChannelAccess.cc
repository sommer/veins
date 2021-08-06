//
// Copyright (C) 2004 Telecommunication Networks Group (TKN) at Technische Universitaet Berlin, Germany.
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

// author:      Marc Loebbers
// part of:     framework implementation developed by tkn
// description: - Base class for physical layers
//              - if you create your own physical layer, please subclass
//                from this class and use the sendToChannel() function!!

#include "veins/base/connectionManager/ChannelAccess.h"

#include "veins/base/utils/FindModule.h"
#include "veins/base/modules/BaseWorldUtility.h"
#include "veins/base/connectionManager/BaseConnectionManager.h"

using std::endl;
using namespace veins;

BaseConnectionManager* ChannelAccess::getConnectionManager(cModule* nic)
{
    std::string cmName = nic->hasPar("connectionManagerName") ? nic->par("connectionManagerName").stringValue() : "";
    if (cmName != "") {
        cModule* ccModule = veins::findModuleByPath(cmName.c_str());

        return dynamic_cast<BaseConnectionManager*>(ccModule);
    }
    else {
        throw cRuntimeError("Variable connectionManagerName must be specified");
    }
}

void ChannelAccess::initialize(int stage)
{
    BatteryAccess::initialize(stage);

    if (stage == 0) {
        if (hasPar("antennaOffsetX")) {
            antennaOffset.x = par("antennaOffsetX").doubleValue();
        }

        if (hasPar("antennaOffsetY")) {
            antennaOffset.y = par("antennaOffsetY").doubleValue();
        }

        if (hasPar("antennaOffsetZ")) {
            antennaOffset.z = par("antennaOffsetZ").doubleValue();
        }

        if (hasPar("antennaOffsetYaw")) {
            antennaOffsetYaw = par("antennaOffsetYaw").doubleValue();
        }

        findHost()->subscribe(BaseMobility::mobilityStateChangedSignal, this);

        cModule* nic = getParentModule();
        cc = getConnectionManager(nic);
        if (cc == nullptr) throw cRuntimeError("Could not find connectionmanager module");
        isRegistered = false;
    }

    usePropagationDelay = par("usePropagationDelay");
}

void ChannelAccess::sendToChannel(cPacket* msg)
{
    EV_TRACE << "sendToChannel: sending to gates\n";

    const auto& gateList = cc->getGateList(getParentModule()->getId());

    for (auto&& entry : gateList) {
        const auto gate = entry.second;
        const auto propagationDelay = calculatePropagationDelay(entry.first);

        if (useSendDirect) {
            if (gate->isVector()) {
                for (int gateIndex = gate->getBaseId(); gateIndex < gate->getBaseId() + gate->size(); gateIndex++) {
                    sendDirect(msg->dup(), propagationDelay, msg->getDuration(), gate->getOwnerModule(), gateIndex);
                }
            }
            else {
                sendDirect(msg->dup(), propagationDelay, msg->getDuration(), gate->getOwnerModule(), gate->getBaseId());
            }
        }
        else {
            sendDelayed(msg->dup(), propagationDelay, gate);
        }
    }
    // Original message no longer needed, copies have been sent to all possible receivers.
    delete msg;
}

simtime_t ChannelAccess::calculatePropagationDelay(const NicEntry* nic)
{
    if (!usePropagationDelay) return 0;

    ChannelAccess* const senderModule = this;
    ChannelAccess* const receiverModule = nic->chAccess;
    // const simtime_t_cref sStart         = simTime();

    ASSERT(senderModule);
    ASSERT(receiverModule);

    /** claim the Move pattern of the sender from the Signal */
    Coord senderPos = senderModule->antennaPosition.getPositionAt();
    Coord receiverPos = receiverModule->antennaPosition.getPositionAt();

    // this time-point is used to calculate the distance between sending and receiving host
    return receiverPos.distance(senderPos) / BaseWorldUtility::speedOfLight();
}

void ChannelAccess::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    if (signalID == BaseMobility::mobilityStateChangedSignal) {
        ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);

        auto heading = Heading::fromCoord(mobility->getCurrentOrientation());
        antennaPosition = AntennaPosition(getId(), mobility->getPositionAt(simTime()) + antennaOffset.rotatedYaw(-heading.getRad()), mobility->getCurrentSpeed(), simTime());
        antennaHeading = Heading(heading.getRad() + antennaOffsetYaw);

        if (isRegistered) {
            cc->updateNicPos(getParentModule()->getId(), antennaPosition.getPositionAt(), antennaHeading);
        }
        else {
            // register the nic with ConnectionManager
            // returns true, if sendDirect is used
            useSendDirect = cc->registerNic(getParentModule(), this, antennaPosition.getPositionAt(), antennaHeading);
            isRegistered = true;
        }
    }
}
