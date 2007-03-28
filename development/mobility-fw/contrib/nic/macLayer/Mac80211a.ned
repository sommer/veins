//***************************************************************************
//* file:        Mac80211a.ned
//*
//* author:      Thomas Freitag, Marc Löbbers
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
//* description: MAC layer class for 802.11a
//**************************************************************************/


//
// Implementation of the 802.11a MAC protocol. This module is intended
// to be used in combination with the SnrEval80211a and Decider80211a
// modules as physical layer.
//
// This module supports ad-hoc mode only (does not generate or handle
// management frames), and does not support fragmentation, RTS threshold,
// duplication detection and recovery.
//
simple Mac80211a
    parameters:
        debug: bool, // debug switch
        queueLength: numeric const,
        rtsCtsThreshold: numeric const,
	fragmentationThreshold: numeric const,

	busyRSSI: numeric const, // minimum RSSI for busy medium
        bitrate: numeric const,
	defaultChannel: numeric const, // minimum channel support

	// auto bit rate selection support: specify borders at which
        // to change modulation.	
	// TODO // TF
	autoBitrate: bool,         // use auto bit rate adaptation

	useOFDMAextensions: bool, 	// use extension defined by pg

	// keep information on neighborhood
	neighborhoodCacheSize: numeric const,	
	// consider information in cache outdate if older than 
	neighborhoodCacheMaxAge: numeric const; 
	
    gates:
        in: uppergateIn;
        out: uppergateOut;
        in: lowergateIn;
        out: lowergateOut;
	out: upperControlOut; // for control messages to upper layer
	in: lowerControlIn;   // for control messages from phy layer
	out: toAssignment;
	in: fromAssignment;
	out: toSink;
endsimple
