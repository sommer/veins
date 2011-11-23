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

package org.mixim.tests.nicTest;

import org.mixim.base.connectionManager.ConnectionManager;
import org.mixim.base.modules.BaseWorldUtility;

network NicTest
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        double numHosts; // total number of hosts in the network
        double run; // total number of hosts in the network
        bool showPassed;

        @display("bgb=$playgroundSizeX,$playgroundSizeY,white,,;bgp=10,10");
    submodules:
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;

        }
        testManager: NicTests;
        channelcontrol: ConnectionManager {
            parameters:
                @display("p=60,30;i=eye14");

        }
        node[numHosts]: TestHost {
            parameters:
                @display("p=50,50;i=laptop");

        }
    connections allowunconnected:
                         // all connections and gates are to be generated dynamically
}
