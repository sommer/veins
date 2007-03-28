//***************************************************************************
// * file:        Network.ned
// *
// * author:      Thomas Freitag, Marc Loebbers
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
// * changelog:   $Revision: 1.2 $
// *              last modified:   $Date: 2007/02/05 17:04:24 $
// *              by:              $Author: tf $
// **************************************************************************/

import
    "Host",
    "ChannelControl",
    "DataSink";


module Sim
    parameters:
        // parametes for the framework
        playgroundSizeX : numeric const,
	playgroundSizeY : numeric const,
        numHosts : numeric const;
        

    submodules:
        channelcontrol: ChannelControl
            parameters:
                playgroundSizeX = playgroundSizeX,
		playgroundSizeY = playgroundSizeY;
            display: "p=50,25;i=eye14";
        host: Host[numHosts]
            display: "p=50,50;i=node";
	sink: DataSink
	    gatesizes:
	        toSink[numHosts];
	    display: "p=75,25,i=eye14";

    connections nocheck:
    // all connections and gates are to be generated dynamically
    for j=0..numHosts-1 do
        host[j].toSink --> sink.toSink[j];
    endfor;

    display: "p=0,0;b=$playgroundSizeX,$playgroundSizeY,rect;o=white";
endmodule




network sim : Sim
    parameters:
        playgroundSizeX = input(40,"playgroundSizeX"),
	playgroundSizeY = input(40,"playgroundSizeY"),
        numHosts =input(2,"Number of hosts:");

endnetwork
