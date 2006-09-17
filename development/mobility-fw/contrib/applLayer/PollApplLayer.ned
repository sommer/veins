//****************************************************************************
// * file:        PollApplLayer.ned
// *
// * author:      Daniel Willkomm
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
// * description: application layer for a simple base station, which polls all 
// *              clients in a round-robin manner
// ***************************************************************************
// * changelog:   $Revision: 347 $
// *              last modified:   $Date: 2006-07-03 17:43:49 +0200 (Mo, 03 Jul 2006) $
// *              by:              $Author: willkomm $
// ***************************************************************************/


// Application layer to test centralized scenarios
//
// Application layer for a base station in a centralized WLAN-like network
// Every 'broadcastInterval' it broadcasts a ping message to determine the 
// reachable clients. Afterwards it polls all reachable clients in a round-robin
// manner, where 'pollTimeout' is the reply timeout for the clients
//
// @sa ClientApplLayer
// @author Daniel Willkomm
simple PollApplLayer
    parameters:
	debug : bool, // debug switch
	numClients: numeric const, // number of clients in the network
	headerLength: numeric const, // length of the application message header (in bits)
	pollTimeout: numeric const, //timeout for poll messages
	broadcastInterval: numeric const; // time between bradcast messages in seconds
    gates:
	in: lowergateIn, // from network layer
        lowerControlIn;
	out: lowergateOut; // to network layer
endsimple
