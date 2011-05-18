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

package org.mixim.modules.nic;

import org.mixim.modules.phy.PhyLayerBattery;
import org.mixim.modules.mac.BMacLayer;

//
// This module implements a BMAC network interface card using
// the "BMACLayer" MAC layer module
//
module NicBMAC like INic
{
    parameters:
        double sleepCurrent @unit(mA); // 20 µA
		double rxCurrent @unit(mA);
		double decodingCurrentDelta @unit(mA);
		double txCurrent @unit(mA);
		double setupRxCurrent @unit(mA);
		double setupTxCurrent @unit(mA);
		double rxTxCurrent @unit(mA);
		double txRxCurrent @unit(mA);
		
    gates:
        input upperGateIn; // to upper layers
        output upperGateOut; // from upper layers
        output upperControlOut; // control information 
        input upperControlIn; // control information 
		input radioIn; // radioIn gate for sendDirect

    submodules:
        mac: BMacLayer {
            @display("p=96,87;i=block/layer");
        }
        
        phy: PhyLayerBattery {
            @display("p=106,157;i=block/process_s");
        }

    connections:
        mac.upperGateOut --> { @display("ls=black;m=m,25,50,25,0"); } --> upperGateOut;
        mac.upperGateIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- upperGateIn;
        mac.upperControlOut --> { @display("ls=red;m=m,75,50,75,0"); } --> upperControlOut;
        mac.upperControlIn <-- { @display("ls=red;m=m,85,0,85,0"); } <-- upperControlIn;

        phy.upperGateOut --> { @display("ls=black;m=m,25,50,25,0"); } --> mac.lowerGateIn;
        phy.upperGateIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- mac.lowerGateOut;
        phy.upperControlOut --> { @display("ls=red;m=m,75,50,75,0"); } --> mac.lowerControlIn;
        phy.upperControlIn <-- { @display("ls=red;m=m,85,0,85,0"); } <-- mac.lowerControlOut;

        radioIn --> phy.radioIn;

}

