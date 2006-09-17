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
//* description: network to test blackboard
//*
//***************************************************************************
//* changelog:   $Revision: 1.2 $
//*              last modified:   $Date: 2004/02/09 13:59:33 $
//*              by:              $Author: omfw-willkomm $
//***************************************************************************


import
    "BasicHost";

module Network
    parameters:
        // parametes for the framework
        playgroundSizeX : numeric const,
        playgroundSizeY : numeric const,
        numHosts : numeric const;
        

    submodules:
        host: BasicHost[numHosts];
            parameters:
                numHosts = numHosts;
            display: "p=50,50;i=node";

    connections nocheck:
    // all connections and gates are to be generated dynamically

    display: "p=0,0;b=$playgroundSizeX,$playgroundSizeY,rect;o=white";
endmodule




network sim : Network
    parameters:
        numHosts =input(2,"Number of hosts:");

endnetwork
