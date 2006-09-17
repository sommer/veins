//***************************************************************************
//* file:        CSMAMacLayer.ned
//*
//* author:      Marc Löbbers
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
//* description: MAC layer class for the CSMAMac
//**************************************************************************/


//
// CSMA MAC protocol
//
simple CSMAMacLayer
    parameters:
        debug: bool, // debug switch
        // level at which we consider medium busy
	busyRSSI: numeric const, 
	// how long is a slot?
	slotDuration: numeric const, 
	// maximum time between a packet and its ack
	difs: numeric const, 
	// maximum number of transmission attempts
	maxTxAttempts: numeric const, 
	// length of MAC header
        headerLength: numeric const,
	// length of the MAC queue
        queueLength: numeric const,
	// default channel 
        defaultChannel : numeric const,
	// bit rate 
	bitrate : numeric const,
	// contention window
	contentionWindow : numeric const;

    gates:
        in: uppergateIn;
        out: uppergateOut;
        in: lowergateIn;
        out: lowergateOut;
	out: upperControlOut; // for control messages to upper layer
	in: lowerControlIn;   // for control messages from phy layer
endsimple
