//***************************************************************************
// * file:        Nic80211.ned
// *
// * author:      Marc Loebbers
// *
// * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
// *              Technische Universitaet Berlin, Germany.
// *
// *              This program is free software; you can redistribute it
// *              and/or modify it under the terms of the GNU General Public
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later
// *              version.
// *              For further information see file COPYING
// *              in the top level directory
// ***************************************************************************
// * part of:     framework implementation developed by tkn
// **************************************************************************/
//#
//# 
//# Energy Framework for Omnet++, version 0.9
//#
//# Author:  Laura Marie Feeney 
//#
//######################################################################
// Based on Nic80211 (above)
//
// Extends Nic80211 to use the Energy Framework to record battery
// consumption.  The relevant changes for communicating with the
// Battery module are in snrEval; core 80211 functionality is not
// changed.

// parameters: note that the NIC module owns the hardware parameters
// to allow external access/alternate implementations.
//
//      {sleep,idle,rx,tx}Current: respective current in mA 
package org.mixim.modules.nic;

import org.mixim.modules.mac.Mac80211;
import org.mixim.modules.phy.PhyLayerBattery;
//import Decider80211;
//import SnrEval80211Battery;

module Nic80211Battery
{
    parameters:
        double sleepCurrent;
        double idleCurrent;
        double rxCurrent;
        double txCurrent;

    gates:
        input uppergateIn; // to upper layers
        output uppergateOut; // from upper layers
        output upperControlOut; // control information 
        output batteryOut; // to battery 
        input radioIn; // radioIn gate for sendDirect

    submodules:
        mac: Mac80211 {
            parameters:
                @display("p=96,87;i=block/layer");
        }
        phy: PhyLayerBattery {
            parameters:
                @display("p=106,157;i=block/process_s");
        }
//        snrEval: SnrEval80211Battery {
//            parameters:
//                @display("p=96,236;i=block/wrxtx");
//        }
//        radio: SingleChannelRadio;
    connections:
        radioIn --> phy.radioIn;
        phy.upperGateIn <-- { @display("m=m,25,0,25,0"); } <-- mac.lowerGateOut;
        phy.upperGateOut --> { @display("m=m,73,0,50,50"); } --> mac.lowerGateIn;
        phy.upperControlOut --> mac.lowerControlIn;
        phy.upperControlOut --> mac.lowerControlIn;

        mac.upperGateOut --> uppergateOut;
        mac.upperControlOut --> upperControlOut;
        mac.upperGateIn <-- uppergateIn;
}

