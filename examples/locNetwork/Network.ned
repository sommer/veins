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

import
    "Node",
    "BaseObject",
    "BaseWorldUtility",
    "ConnectionManager";

module Sim
    parameters:
        playgroundSizeX: numeric const, // x size of the area the nodes are in (in meters)
        playgroundSizeY: numeric const, // y size of the area the nodes are in (in meters)
        playgroundSizeZ: numeric const, // z size of the area the nodes are in (in meters)
        numObjects: numeric const, // total number of objects in the network
	numHosts : numeric const,
	propagationType: string;

    submodules:
	propagationmodel: propagationType like ConnectionManager;
		display: "p=50,25;i=eye14";
	world: BaseWorldUtility;
	    parameters:
                playgroundSizeX = playgroundSizeX,
                playgroundSizeY = playgroundSizeY,
                playgroundSizeZ = playgroundSizeZ;
	node: LocNode[numHosts];
		display: "p=50,50;i=node";
        object: BaseObject[numObjects];
            display: "b=40,24;p=164,114";

    connections nocheck:
    // all connections and gates are to be generated dynamically

    display:
	"p=0,0;o=white";
endmodule

network sim : Sim
endnetwork
