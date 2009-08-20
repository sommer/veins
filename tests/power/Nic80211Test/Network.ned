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
package org.mixim.tests.power.Nic80211Test;

import org.mixim.base.connectionManager.ConnectionManager;
import org.mixim.base.modules.BaseWorldUtility;


module Sim
{
    parameters:
        // parametes for the framework
        double playgroundSizeX @unit(m);
        double playgroundSizeY @unit(m);
        double numHosts;


        @display("bgb=$playgroundSizeX,$playgroundSizeY,white,,;bgp=0,0");
    submodules:
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
        }
        channelcontrol: ConnectionManager {
            parameters:
                @display("p=50,25;i=eye14");
        }
        host[numHosts]: Host {
            parameters:
                @display("p=50,50;i=node");

        }
    connections allowunconnected:
    // all connections and gates are to be generated dynamically

}




network sim extends Sim
{
    parameters:
        playgroundSizeX @prompt("playgroundSizeX") = default(40m);
        playgroundSizeY @prompt("playgroundSizeY") = default(40m);
        numHosts @prompt("Number of hosts:") = default(2);

}
