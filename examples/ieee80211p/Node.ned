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

package org.mixim.examples.ieee80211p;
import org.mixim.modules.mobility.traci.TraCIMobility;

import org.mixim.modules.application.ieee80211p.TestWaveApplLayer;
import org.mixim.base.modules.BaseArp;

import org.mixim.modules.nic.Nic80211p;


module Node
{
    parameters:
        @display("p=10,10;b=4,4,rect;o=white");

    gates:
        input radioIn; // gate for sendDirect

    submodules:
        appl: TestWaveApplLayer {
            @display("p=90,50;b=100,20,rect");
        }

        arp: BaseArp {
            @display("p=257,28;b=30,25");
        }

        nic: Nic80211p {
            @display("p=90,150;b=100,20,rect");
        }

        mobility: TraCIMobility {
            @display("p=130,172;i=block/cogwheel");
        }
    connections:
        nic.upperLayerOut --> appl.lowerLayerIn;
        nic.upperLayerIn <-- appl.lowerLayerOut;
        nic.upperControlOut --> appl.lowerControlIn;
        nic.upperControlIn <-- appl.lowerControlOut;

        radioIn --> nic.radioIn;
}
