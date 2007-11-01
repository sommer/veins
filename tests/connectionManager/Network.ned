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

import  
    "BasicHost", 
    "ConnectionManager",
    "BaseWorldUtility"; 
 
// @author Karl Wessel
module Network 
    parameters: 
        playgroundSizeX : numeric const, // x size of the area the nodes are in (in meters)
        playgroundSizeY : numeric const, // y size of the area the nodes are in (in meters)
        playgroundSizeZ : numeric const, // z size of the area the nodes are in (in meters)
        numHosts : numeric const; // total number of hosts in the network
                                  
    submodules: 
    	world: BaseWorldUtility;
    		parameters: 
                playgroundSizeX = playgroundSizeX, 
                playgroundSizeY = playgroundSizeY,
                playgroundSizeZ = playgroundSizeZ;
        channelcontrol: ConnectionManager; 
            
            display: "p=60,30;i=eye14";
        node: BasicHost[numHosts]; 
            parameters: 
                numHosts = numHosts; 
            display: "p=50,50;i=laptop";
    connections nocheck: 
                         // all connections and gates are to be generated dynamically
                         
    display: "p=10,10;b=$playgroundSizeX,$playgroundSizeY,rect;o=white";
endmodule 


// basic example simulation for the Mobility Framework
network ccSim : Network
endnetwork 

