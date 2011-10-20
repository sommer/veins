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

package org.mixim.examples.ieee802154Narrow;

//
// TODO auto-generated module
//
simple NetworkStackTrafficGen
{
    parameters:
        double packetTime @unit(s);
        double packetsPerPacketTime;
        bool debug; // debug switch
        int burstSize = default(1);
    	int packetLength @unit("bit"); // length of the generated packet (in bits)
    	int destination = default(-1);
    gates:
        input lowerLayerIn; // from NIC
        output lowerLayerOut; // to NIC
        input lowerControlIn; // control from NIC
        output lowerControlOut; // control to NIC
}
