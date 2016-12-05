//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package org.car2x.veins.modules.nic;

import org.car2x.veins.modules.phy.PhyLayer80211p;
import org.car2x.veins.modules.mac.aribt109.IAribT109Mac;

//
// This NIC implements an ARIB T109 network interface card.
//
// @author Julian Heinovski
//
// @see AribT109Mac
// @see PhyLayer80211p
//
module AribT109Nic like INic80211p
{
    @display("bgb=139,182;bgl=2");
    gates:
        input upperLayerIn; // to upper layers
        output upperLayerOut; // from upper layers
        output upperControlOut; // control information
        input upperControlIn; // control information
        input radioIn; // radioIn gate for sendDirect

    submodules:
        phy80211p: PhyLayer80211p {
            @display("p=70,127;i=block/process_s");
        }

        aribT109Mac: <default("AribT109MobileStationMacLayer")> like org.car2x.veins.modules.mac.aribt109.IAribT109Mac {

            @display("p=70,58;is=s");
        }

    connections:
        radioIn --> phy80211p.radioIn;

        aribT109Mac.lowerControlOut --> phy80211p.upperControlIn;
        aribT109Mac.lowerLayerOut --> phy80211p.upperLayerIn;
        phy80211p.upperLayerOut --> aribT109Mac.lowerLayerIn;
        phy80211p.upperControlOut --> aribT109Mac.lowerControlIn;

        aribT109Mac.upperControlIn <-- upperControlIn;
        aribT109Mac.upperLayerIn <-- upperLayerIn;
        aribT109Mac.upperLayerOut --> upperLayerOut;
        aribT109Mac.upperControlOut --> upperControlOut;
}
