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

#include "veins_inet/VeinsInetManager.h"
#include "veins/base/utils/Coord.h"
#include "veins_inet/VeinsInetMobility.h"

using Veins::VeinsInetManager;

Define_Module(Veins::VeinsInetManager);

VeinsInetManager::~VeinsInetManager() {
}

void VeinsInetManager::preInitializeModule(cModule* mod, const std::string& nodeId, const Coord& position, const std::string& road_id, double speed, double angle, VehicleSignal signals) {
	// pre-initialize VeinsInetMobility
	for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
		cModule* submod = SUBMODULE_ITERATOR_TO_MODULE(iter);
		VeinsInetMobility* inetmm = dynamic_cast<VeinsInetMobility*>(submod);
		if (!inetmm) return;
		inetmm->preInitialize(nodeId, inet::Coord(position.x, position.y), road_id, speed, angle);
	}
}

void VeinsInetManager::updateModulePosition(cModule* mod, const Coord& p, const std::string& edge, double speed, double angle, VehicleSignal signals) {
	// update position in VeinsInetMobility
	for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
		cModule* submod = SUBMODULE_ITERATOR_TO_MODULE(iter);
		VeinsInetMobility *inetmm = dynamic_cast<VeinsInetMobility*>(submod);
		if (!inetmm) return;
		inetmm->nextPosition(inet::Coord(p.x, p.y), edge, speed, angle);
	}
}



