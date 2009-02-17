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


module Sim
{
    parameters:
        double playgroundSizeX; // x size of the area the nodes are in (in meters)
        double playgroundSizeY; // y size of the area the nodes are in (in meters)
        double playgroundSizeZ; // z size of the area the nodes are in (in meters)
        double numHosts; // total number of hosts in the network  
        
        @display("p=0,0;b=$playgroundSizeX,$playgroundSizeY,rect;o=white");      

    submodules:
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=100,0;i=misc/globe");

        }
        channelcontrol: ConnectionManager {
            parameters:
            	@display("p=50,25;i=eye14");
        }
        host[numHosts]: Host {
            @display("p=20,20;b=20,20,rect,yellow,,;i=device/wifilaptop");
        }

    connections allowunconnected:
    // all connections and gates are to be generated dynamically

}




network sim extends Sim
{
    parameters:

}
