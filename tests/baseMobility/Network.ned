//***************************************************************************
//* file:        Network.ned
//*
//* author:      Daniel Willkomm
//*
//* copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
//*              Technische Universitaet Berlin, Germany.
//*
//*              This program is free software; you can redistribute it 
//*              and/or modify it under the terms of the GNU General Public 
//*              License as published by the Free Software Foundation; either
//*              version 2 of the License, or (at your option) any later 
//*              version.
//*              For further information see file COPYING 
//*              in the top level directory
//***************************************************************************
//* part of:     framework implementation developed by tkn
//* description: A test networl for cc connections
//*
//***************************************************************************
//* changelog:   $Revision: 80 $
//*              last modified:   $Date: 2004-07-23 20:05:13 +0200 (Fr, 23 Jul 2004) $
//*              by:              $Author: omfw-willkomm $
//***************************************************************************


//***************************************************************************
//*
//* edited by Michael Swigulski for testing purposes
//*
//***************************************************************************


package org.mixim.tests.baseMobility;

import org.mixim.base.modules.BaseWorldUtility;


// @author Daniel Willkomm
module Network
{
    parameters:
        double playgroundSizeX; // x size of the area the nodes are in (in meters)
        double playgroundSizeY; // y size of the area the nodes are in (in meters)
        double playgroundSizeZ; // z size of the area the nodes are in (in meters)
        double numHosts; // total number of hosts in the network

        @display("bgb=$playgroundSizeX,$playgroundSizeY,white,,;bgp=10,10");
    submodules:
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
        }
        //      channelcontrol: ConnectionManager; 
        //            display: "p=60,30;i=eye14";
        node[numHosts]: TestHost {
            parameters:
                numHosts = numHosts;
                @display("p=50,50;i=laptop");
        }
        node2[numHosts]: CrashHost {
            parameters:
                numHosts = numHosts;
                @display("p=50,50;i=laptop");
//    connections nocheck: 
// all connections and gates are to be generated dynamically

        }
}


// basic example simulation for the Mobility Framework
network bmSim extends Network
{
    parameters:
}

