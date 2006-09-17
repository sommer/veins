//***************************************************************************
//* file:        FloodNetwork.ned
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
//* description: FloodNetwork Compound Module
//*
//***************************************************************************
//* changelog:   $Revision: 103 $
//*              last modified:   $Date: 2004-09-23 09:56:00 +0200 (Do, 23 Sep 2004) $
//*              by:              $Author: omfw-willkomm $
//***************************************************************************

import
    "FloodHost",
    "ChannelControl";

/// @interface   testNet
/// @brief Compound module that forms a very simple network consisting
///    of the basic modules of each layer. It was implemented for testing
///    purposes and can be taken as an example of how to build a network
///    in this legendary framework
///
///
/// @author Marc Löbbers
/// @ingroup networks
///


module FloodNetwork
    parameters:
        // parametes for the framework
        playgroundSizeX : numeric const,
	playgroundSizeY : numeric const,
        numHosts : numeric const;
        

    submodules:
        channelcontrol: ChannelControl;
            parameters:
                playgroundSizeX = playgroundSizeX,
		playgroundSizeY = playgroundSizeY;
            display: "p=50,25;i=eye14";
        host: FloodHost[numHosts];
            display: "p=50,50;i=node";

    connections nocheck:
    // all connections and gates are to be generated dynamically

    display: "p=5,5;b=$playgroundSizeX,$playgroundSizeY,rect;o=white";
endmodule




network floodSim : FloodNetwork
    parameters:
        playgroundSizeX = input(40,"playgroundSizeX"),
	playgroundSizeY = input(40,"playgroundSizeY"),
        numHosts =input(2,"Number of hosts:");

endnetwork
