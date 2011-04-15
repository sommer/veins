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
simple Mac80211 extends BaseMacLayer
{
    parameters:
        @class(Mac80211);
        // enable debugging for this module
        bool debug = default(false);
        
        int queueLength;
        // Number of bits in a packet before RTS/CTS is used. 
        // Is currently not used.
        int rtsCtsThreshold = default(-1); 

        double bitrate @unit(bps);

    	// auto bit rate selection support: specify borders at which
        // to change modulation.	
    	bool autoBitrate;         // use auto bit rate adaptation
    	double snr2Mbit @unit(dB);  // threshold to use 2Mbit (in dB)
    	double snr5Mbit @unit(dB);  // threshold to use 5.5Mbit (in dB)
    	double snr11Mbit @unit(dB);  // threshold to use 11Mbit (in dB)

    	// keep information on neighborhood
    	int neighborhoodCacheSize;	
    	// consider information in cache outdate if older than (in seconds)
    	double neighborhoodCacheMaxAge @unit(s); 
    	//the power to transmit packets with [mW]
    	double txPower @unit(mW);
    	
    	headerLength = default(272bit);
}
