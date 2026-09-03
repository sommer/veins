//
// Copyright (C) 2008 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
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

package veins.nodes;

import veins.base.modules.*;
import veins.modules.nic.Nic80211p;

module Car
{
    parameters:
        string applType; //type of the application layer
        string nicType = default("Nic80211p"); // type of network interface card
        string veinsmobilityType = default("veins.modules.mobility.traci.TraCIMobility"); //type of the mobility module
    gates:
        input veinsradioIn; // gate for sendDirect
    submodules:
        appl: <applType> like veins.base.modules.IBaseApplLayer {
            parameters:
                @display("p=60,50");
        }

        nic: <nicType> like veins.modules.nic.INic80211p {
            parameters:
                @display("p=60,166");
        }

        veinsmobility: <veinsmobilityType> like veins.base.modules.IMobility {
            parameters:
                @display("p=130,172;i=block/cogwheel");
        }

        bridge: veins.modules.application.ieee80211p.VanetzaToVeinsBridge {
            parameters:
                @display("p=130,50;i=block/network2");
        }

        converter: veins.modules.application.ieee80211p.ApplicationToVanetzaConverter {
            parameters:
                @display("p=200,50");
        }

    connections:
        // =========================
        // DATA PLANE
        // =========================

        // before: appl.lowerLayerOut --> nic.upperLayerIn;
        appl.lowerLayerOut --> bridge.upperLayerIn;
        bridge.lowerLayerOut --> nic.upperLayerIn;


        // before: nic.upperLayerOut --> appl.lowerLayerIn;
        nic.upperLayerOut --> bridge.lowerLayerIn;
        bridge.upperLayerOut --> appl.lowerLayerIn;


        // =========================
        // CONTROL PLANE
        // =========================

        // before: nic.upperControlIn <-- appl.lowerControlOut;
        appl.lowerControlOut --> bridge.upperControlIn;
        bridge.lowerControlOut --> nic.upperControlIn;


        // before: nic.upperControlOut --> appl.lowerControlIn;
        nic.upperControlOut --> bridge.lowerControlIn;
        bridge.upperControlOut --> appl.lowerControlIn;


        // =========================
        // RADIO
        // =========================

        // before: veinsradioIn --> nic.radioIn;
        veinsradioIn --> nic.radioIn;
}
