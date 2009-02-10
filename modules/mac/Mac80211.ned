//***************************************************************************
//* file:        Mac80211.ned
//*
//* author:      Marc L�bbers
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
//* description: MAC layer class for 802.11
//**************************************************************************/

package org.mixim.modules.mac;

//
// Implementation of the 802.11b MAC protocol. This module is intended
// to be used in combination with the Decider80211 as decider for the 
// physical layer.
//
// This module supports ad-hoc mode only (does not generate or handle
// management frames), and does not support fragmentation, RTS threshold,
// duplication detection and recovery.
//
simple Mac80211
{
    parameters:
        bool coreDebug; // debug switch
        int queueLength;
        int rtsCtsThreshold;

        double bitrate;
		int defaultChannel; // minimum channel support

    	// auto bit rate selection support: specify borders at which
            // to change modulation.	
    	bool autoBitrate;         // use auto bit rate adaptation
    	double snr2Mbit;  // threshold to use 2Mbit (in dB)
    	double snr5Mbit;  // threshold to use 5.5Mbit (in dB)
    	double snr11Mbit;  // threshold to use 11Mbit (in dB)

    	// keep information on neighborhood
    	int neighborhoodCacheSize;	
    	// consider information in cache outdate if older than 
    	double neighborhoodCacheMaxAge; 
    	//the power to transmit packets with
    	double txPower;
	
    gates:
        input upperGateIn;
        output upperGateOut;
        input lowerGateIn;
        output lowerGateOut;
		output upperControlOut; // for control messages to upper layer
		input lowerControlIn;   // for control messages from phy layer
}
