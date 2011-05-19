//***************************************************************************
// * file:        Network.ned
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
// * description: a network to test the 802.11 modules
// *              
// ***************************************************************************
// * changelog:   $Revision: 103 $
// *              last modified:   $Date: 2004-09-23 09:56:00 +0200 (Do, 23 Sep 2004) $
// *              by:              $Author: omfw-willkomm $
// **************************************************************************/
package org.mixim.examples.Mac80211;

import org.mixim.base.connectionManager.ConnectionManager;
import org.mixim.base.modules.BaseWorldUtility;
import org.mixim.modules.node.Host80211;
//
//This example shows how to use MiXiMs 802.11 protocol implementation. 
//
//The network consists of ten hosts broadcasting bursts of three packets
//to the other hosts. When a broadcast is received a reply is sent back 
//to the sender.
//
//The hosts use "Mac80211" as MAC layer and their phy layer uses 
//"SimplePathlossModel" as AnalogueModel and "Decider80211" as Decider.
//Note that Mac80211 and Decider80211 are always meant to be used
//together.
//
//The default configuration in omnetpp.ini uses no propagation delay.
//
network Mac80211
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        double numHosts; // total number of hosts in the network  

        @display("bgb=$playgroundSizeX,$playgroundSizeY,white,,;bgp=10,50");

    submodules:
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=171,27;i=misc/globe");

        }
        connectionManager: ConnectionManager {
            parameters:
                @display("p=66,25;b=42,42,rect,green;i=abstract/multicast");
        }
        host[numHosts]: Host80211 {
            netwType = "BaseNetwLayer";
            applType = "BurstApplLayer";
            mobType = "ConstSpeedMobility";
        }

    connections allowunconnected:
    // all connections and gates are to be generated dynamically

}
