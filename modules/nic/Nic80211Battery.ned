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

package org.mixim.modules.nic;

import org.mixim.modules.mac.Mac80211;
import org.mixim.modules.phy.PhyLayerBattery;

//
// Based on Nic80211
//
// Extends Nic80211 to use the Energy Framework to record battery
// consumption.  The relevant changes for communicating with the
// Battery module are in PhyLayerBattery; core 80211 functionality 
// is not changed.
//
// parameters: note that the NIC module owns the hardware parameters
// to allow external access/alternate implementations.
//
module Nic80211Battery like INic
{
    parameters:
        string connectionManagerName = default("");   //name of the ConnectionManager module
        double sleepCurrent @unit(mA);
        double rxCurrent @unit(mA);
        double decodingCurrentDelta @unit(mA);
        double txCurrent @unit(mA);
        double setupRxCurrent @unit(mA);
        double setupTxCurrent @unit(mA);
        double rxTxCurrent @unit(mA);
        double txRxCurrent @unit(mA);

        @display("i=block/ifcard");
    gates:
        input upperGateIn; // to upper layers
        output upperGateOut; // from upper layers
        output upperControlOut; // control information 
        input upperControlIn; // control information 
        input radioIn; // radioIn gate for sendDirect

    submodules:
        mac: Mac80211 {
            parameters:
                @display("p=96,87;i=block/layer");
        }
        phy: PhyLayerBattery {
            parameters:
                headerLength = 192bit;
                @display("p=106,157;i=block/process_s");
        }

    connections:
        radioIn --> phy.radioIn;
        phy.upperGateIn <-- { @display("m=m,25,0,25,0"); } <-- mac.lowerGateOut;
        phy.upperGateOut --> { @display("m=m,73,0,50,50"); } --> mac.lowerGateIn;
        phy.upperControlOut --> mac.lowerControlIn;
        phy.upperControlIn <-- mac.lowerControlOut;

        mac.upperGateOut --> upperGateOut;
        mac.upperControlOut --> upperControlOut;
        mac.upperGateIn <-- upperGateIn;
        mac.upperControlIn <-- upperControlIn;
}

