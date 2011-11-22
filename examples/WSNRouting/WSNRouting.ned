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

package org.mixim.examples.WSNRouting;


import org.mixim.base.modules.BaseNetwork;
import org.mixim.modules.node.Host802154_2400MHz;

//
// This example shows how to simulate wireless sensor networks.
// Three different routing protocols are considered, for different traffic 
// types:
// convergecast, with Wiseroute;
// network-level broadcast, with flooding;
// and probabilistic broadcast.
//
network WSNRouting extends BaseNetwork
{
    parameters:
        int numHosts; // total number of hosts in the network

    submodules:
        node[numHosts]: Host802154_2400MHz {
            parameters:
                numHosts = numHosts;
                @display("p=158,99;b=42,42,rect,yellow;i=device/wifilaptop");

        }
    connections allowunconnected:
}
