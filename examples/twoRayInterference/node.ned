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

package org.mixim.examples.twoRayInterference;
import org.mixim.modules.mobility.ConstSpeedMobility;
import org.mixim.base.modules.BaseUtility;

import org.mixim.modules.application.WaveApplLayer;
import org.mixim.base.modules.BaseArp;

import org.mixim.modules.nic.Nic80211p;


//
// TODO auto-generated type
//
module Node
{
    parameters:
        @display("p=10,10;b=4,4,rect;o=white");

    gates:
        input radioIn; // gate for sendDirect

    submodules:
        blackboard: BaseUtility {
            @display("p=195,28;b=30,25");
        }

        appl: WaveApplLayer {
            @display("p=90,50;b=100,20,rect");
        }

        arp: BaseArp {
            @display("p=257,28;b=30,25");
        }

        nic: Nic80211p {
            @display("p=90,150;b=100,20,rect");
        }

        constSpeedMobility: ConstSpeedMobility {
            @display("p=223,79");
        }
    connections:
        nic.upperGateOut --> appl.lowerGateIn;
        nic.upperGateIn <-- appl.lowerGateOut;
        nic.upperControlOut --> appl.lowerControlIn;
        nic.upperControlIn <-- appl.lowerControlOut;

        radioIn --> nic.radioIn;
}
