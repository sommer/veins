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

import org.mixim.base.modules.BaseMacLayer;

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
        double headerLength; // length of the MAC packet header (in bits)
        
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
    	//the power to transmit packets with [mW]
    	double txPower;
	
    gates:
        input upperGateIn; // from net layer
        output upperGateOut; // to net layer
        input upperControlIn; // control from net layer
        output upperControlOut; // control to net layer
        input lowerGateIn; // from phy
        output lowerGateOut; // to phy
        input lowerControlIn; // control from phy
        output lowerControlOut; // control to phy
}
