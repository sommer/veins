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

package org.mixim.examples.multiChannel;

import org.mixim.base.connectionManager.ConnectionManager;
import org.mixim.base.modules.BaseWorldUtility;


module Sim
{
    parameters:
        // parameters for the framework
        double playgroundSizeX;
        double playgroundSizeY;
        double playgroundSizeZ;
        double numHosts;

        @display("bgb=$playgroundSizeX,$playgroundSizeY,white,,;bgp=0,0");
    submodules:
        strongChannel: ConnectionManager {
            parameters:
                @display("p=75,25;i=abstract/multicast");
        }
        weakChannel: ConnectionManager {
            parameters:
                @display("p=50,25;i=abstract/multicast");
        }
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
        }
        strongNodes[numHosts]: Node {
            parameters:
                @display("p=50,50;i=device/wifilaptop");

        }
        weakNodes[numHosts]: Node {
            parameters:
                @display("p=75,50;i=device/palm");

        }
    connections allowunconnected:
    // all connections and gates are to be generated dynamically

}




network sim extends Sim
{
    parameters:
        playgroundSizeX @prompt("playgroundSizeX") = default(40);
        playgroundSizeY @prompt("playgroundSizeY") = default(40);
        numHosts @prompt("Number of hosts:") = default(2);

}
