//***************************************************************************
//* file:        P2PNetwork.ned
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
//* description: A basic example p2p network
//*
//***************************************************************************
//* changelog:   $Revision: 103 $
//*              last modified:   $Date: 2004-09-23 09:56:00 +0200 (Thu, 23 Sep 2004) $
//*              by:              $Author: omfw-willkomm $
//***************************************************************************

import
    "P2PHost",
    "ChannelControl";


// A network only consisting of basic modules
//
// The P2PNetwork does not contain any real functionality. It should
// be the basis for all other simulations build with our framework and
// is mainly to help to understand the concept and for all programmers
// as a reference for their own implementations. 
//
// You should use the P2PNetwork if sophisticated interference models
// are not of interest and you prefere a faster
// simulation. Collisions and interference is modeled into the bit
// error rate for these kinds of networks. The advantage is that
// interference doesn't has to be explicitly modeled and thus messages
// are not braodcasted but sent directly to the specified next hop.
//
// P2PHost is a vector of size numHosts (which specifies the number
// of hosts present in the network)
//
// @author Daniel Willkomm
module P2PNetwork
    parameters:
        playgroundSizeX : numeric const, // x size of the area the nodes are in (in meters)
        playgroundSizeY : numeric const, // y size of the area the nodes are in (in meters)
        numHosts : numeric const; // total number of hosts in the network

    submodules:
        channelcontrol: ChannelControl;
            parameters:
                playgroundSizeX = playgroundSizeX,
                playgroundSizeY = playgroundSizeY;
            display: "p=60,30;i=eye14";
        host: P2PHost[numHosts];
            display: "p=50,50;i=laptop";

    connections nocheck:
    // all connections and gates are to be generated dynamically

    display: "p=10,10;b=$playgroundSizeX,$playgroundSizeY,rect;o=white";
endmodule


// example simulation using the P2PHost
network p2pSim : P2PNetwork
endnetwork
