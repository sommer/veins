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
package mixnet.modules;

//
// INET <-> MiXiM compatibility class. Connects INET network layer
// with MiXiM NIC.
// See C++ definition for details.
//
simple MixnetBridge
{
    parameters:
        bool notAffectedByHostState = default(true);

        //The INET MAC-address for this bridge's NIC
        string address = default("auto");
        //The maximum transmission unit for this bridge's NIC
        int mtu = default(1500);

        @display("i=block/classifier2");
    gates:
        input upperGateIn; // from upper layer
        output upperGateOut; // to upper layer

        input lowerGateIn; // from lower layer
        output lowerGateOut; // to lower layer
        input lowerControlIn; // control from lower layer
        output lowerControlOut; // control to lower layer
}
