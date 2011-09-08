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

import org.mixim.base.connectionManager.ConnectionManager;
import org.mixim.base.modules.BaseWorldUtility;
import org.mixim.modules.node.Host80211;

network Network
{
    submodules:
        world: BaseWorldUtility {
            @display("p=70,20");
        }
        connectionManager: ConnectionManager {
            @display("p=70,91");
        }
        node0: Host80211 {
            @display("p=181,160;b=85,78");
            netwType = "BaseNetwLayer";
            applType = "BurstApplLayer";
            mobType = "ConstSpeedMobility";
        }
        node1: Host80211 {
            @display("p=181,160;b=85,78");
            netwType = "BaseNetwLayer";
            applType = "BurstApplLayer";
            mobType = "ConstSpeedMobility";
        }
  
    connections allowunconnected:
}
