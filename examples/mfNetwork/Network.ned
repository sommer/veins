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
    "BaseWorldUtility",
    "BasePropagation";

module Sim
    parameters:
        // parameters for the framework
        playgroundSizeX : numeric const,
		playgroundSizeY : numeric const,
		playgroundSizeZ : numeric const,
        numHosts : numeric const;

    submodules:
        prop: BasePropagation;
            display: "p=50,25;i=eye14";
		world: BaseWorldUtility;
            parameters:
                playgroundSizeX = playgroundSizeX,
				playgroundSizeY = playgroundSizeY,
				playgroundSizeZ = playgroundSizeZ;
        node: Node[numHosts];
            display: "p=50,50;i=node";

    connections nocheck:
    // all connections and gates are to be generated dynamically

    display: "p=0,0;b=$playgroundSizeX,$playgroundSizeY,rect;o=white";
endmodule




network sim : Sim
    parameters:
        playgroundSizeX = input(40,"playgroundSizeX"),
		playgroundSizeY = input(40,"playgroundSizeY"),
        numHosts =input(2,"Number of hosts:");

endnetwork
