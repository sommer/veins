//
// Copyright (C) 2006-2017 Christoph Sommer <sommer@ccs-labs.org>
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

#include "veins_inet/VeinsInetManagerBase.h"

#include "veins/base/utils/Coord.h"
#include "veins_inet/VeinsInetMobility.h"
#include "inet/common/scenario/ScenarioManager.h"

using veins::VeinsInetManagerBase;

Define_Module(veins::VeinsInetManagerBase);

VeinsInetManagerBase::~VeinsInetManagerBase()
{
}

void VeinsInetManagerBase::initialize(int stage)
{
    if (stage != 1)
        return;

#if INET_VERSION >= 0x0402
    signalManager.subscribeCallback(this, TraCIScenarioManager::traciModulePreInitSignal, [this](SignalPayload<cObject*> payload) {
        cModule* module = dynamic_cast<cModule*>(payload.p);
        ASSERT(module);

        cModule* root = getSimulation()->getSystemModule();

        auto* notification = new inet::cPreModuleInitNotification();
        notification->module = module;
        root->emit(POST_MODEL_CHANGE, notification, NULL);
    });
#endif
}

void VeinsInetManagerBase::preInitializeModule(cModule* mod, const std::string& nodeId, const Coord& position, const std::string& road_id, double speed, Heading heading, VehicleSignalSet signals)
{
    TraCIScenarioManager::preInitializeModule(mod, nodeId, position, road_id, speed, heading, signals);

    // pre-initialize VeinsInetMobility
    auto mobilityModules = getSubmodulesOfType<VeinsInetMobility>(mod);
    for (auto inetmm : mobilityModules) {
        inetmm->preInitialize(nodeId, inet::Coord(position.x, position.y), road_id, speed, heading.getRad());
    }
}

void VeinsInetManagerBase::updateModulePosition(cModule* mod, const Coord& p, const std::string& edge, double speed, Heading heading, VehicleSignalSet signals)
{
    TraCIScenarioManager::updateModulePosition(mod, p, edge, speed, heading, signals);

    // update position in VeinsInetMobility
    auto mobilityModules = getSubmodulesOfType<VeinsInetMobility>(mod);
    for (auto inetmm : mobilityModules) {
        inetmm->nextPosition(inet::Coord(p.x, p.y), edge, speed, heading.getRad());
    }
}
