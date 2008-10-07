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

import Node;
import BaseObject;
import BaseWorldUtility;
import ConnectionManager;

module Sim
{
    parameters:
        double playgroundSizeX; // x size of the area the nodes are in (in meters)
        double playgroundSizeY; // y size of the area the nodes are in (in meters)
        double playgroundSizeZ; // z size of the area the nodes are in (in meters)
        double numObjects; // total number of objects in the network
        double numHosts;
        string propagationType;

        @display("bgb=,,white,,;bgp=0,0");
    submodules:
        propagationmodel: <propagationType> like ConnectionManager {
            parameters:
                @display("p=50,25;i=eye14");
        }
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
        }
        node[numHosts]: LocNode {
            parameters:
                @display("p=50,50;i=node");
        }
        object[numObjects]: BaseObject {
            parameters:
                @display("b=40,24;p=164,114");

        }
    connections allowunconnected:
    // all connections and gates are to be generated dynamically

}

network sim extends Sim
{
    parameters:
}
