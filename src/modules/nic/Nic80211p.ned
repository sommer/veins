//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
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

package org.mixim.modules.nic;

import org.mixim.modules.phy.PhyLayer80211p;
import org.mixim.modules.mac.ieee80211p.Mac80211p;
import org.mixim.modules.mac.ieee80211p.Mac1609_4;

module Nic80211p
{
    gates:
        input upperLayerIn; // to upper layers
        output upperLayerOut; // from upper layers
        output upperControlOut; // control information 
        input upperControlIn; // control information 
        input radioIn; // radioIn gate for sendDirect

    submodules:
        phy80211p: PhyLayer80211p {
            @display("p=69,218;i=block/process_s");
        }

        mac80211p: Mac80211p {
            @display("p=69,153");
        }

        mac1609_4: Mac1609_4 {
            @display("p=69,82");
        }

    connections:

        radioIn --> phy80211p.radioIn;

        mac80211p.lowerControlOut --> phy80211p.upperControlIn;
        mac80211p.lowerLayerOut --> phy80211p.upperLayerIn;
        phy80211p.upperLayerOut --> mac80211p.lowerLayerIn;
        phy80211p.upperControlOut --> mac80211p.lowerControlIn;

        mac1609_4.upperControlIn <-- upperControlIn;
        mac1609_4.upperLayerIn <-- upperLayerIn;

        mac1609_4.upperLayerOut --> upperLayerOut;
        mac1609_4.upperControlOut --> upperControlOut;

        mac1609_4.lowerLayerIn <-- mac80211p.upperLayerOut;
        mac1609_4.lowerLayerOut --> mac80211p.upperLayerIn;
        mac1609_4.lowerControlOut --> mac80211p.upperControlIn;
        mac1609_4.lowerControlIn <-- mac80211p.upperControlOut;
}


