package org.mixim.modules.mac;
//***************************************************************************
//* file:        CSMA.ned
//*
//* author:      Jerome Rousselot, Marc Loebbers
//* 
//* copyright:   (C) 2007-2009 CSEM SA, Neuchatel, Switzerland.
//*		 		 (C) 2004 Telecommunication Networks Group (TKN) at
//*              Technische Universitaet Berlin, Germany.
//*
//*              This program is free software; you can redistribute it
//*              and/or modify it under the terms of the GNU General Public
//*              License as published by the Free Software Foundation; either
//*              version 2 of the License, or (at your option) any later
//*              version.
//*              For further information see file COPYING
//*              in the top level directory
//*
//* Funding: This work was partially financed by the European Commission under the  
//* Framework 6 IST Project "Wirelessly Accessible Sensor Populations"  
//* (WASP) under contract IST-034963.
//***************************************************************************
//* part of:    Modifications to the MF-2 framework by CSEM
//**************************************************************************/

import org.mixim.base.modules.BaseMacLayer;

//
// Generic CSMA protocol supporting Mac-ACKs as well as
// constant, linear or exponential backoff times.
//
simple csma extends BaseMacLayer
{
    parameters:
		@class(csma);
        // debug switch
        bool debug = default(false);
        bool stats = default(true);
        bool trace = default(false);

        // length of MAC header
        headerLength @unit(bit) = default(72 bit);
        // size of the MAC queue (maximum number of packets in Tx buffer)
        double queueLength = default(100);
        // bit rate 
        double bitrate @unit(bps) = default(250000 bps);        
          
        // Clear Channel Assessment detection time
        double ccaDetectionTime @unit(s) = default(0.000128 s); // 8 symbols
        // Time to setup radio to reception state
        double rxSetupTime @unit(s);
        // Time to switch radio from Rx to Tx state
        double aTurnaroundTime @unit(s) = default(0.000192 s);    // 12 symbols
        //tx power [mW]
        double txPower @unit(mW);
        
        // Send/Expect MAC acks for unicast traffic?
        bool useMACAcks;
        // maximum number of frame retransmission
        // only used when usage of MAC acks is enabled
        double macMaxFrameRetries = default(3);        
        // time to wait for an acknowledgement after transmitting a unicast frame.
        // only used when usage of MAC acks is enabled
        double macAckWaitDuration @unit(s) = default(0.00056 s);  // 1+12+10+12 symbols
        // complete MAC ack message length (in bits) 
        // (! headerLength is not added to this)
        // only used when usage of MAC acks is enabled
        double ackLength @unit(bit) = default(40 bit);   
        // simple interframe space. Time to wait between receiving a frame and acknowledging it.
        // Should be bigger than the maximum time for switching between Tx and Rx at the receiver.
        // only used when usage of MAC acks is enabled
        double sifs @unit(s) = default(0.000192 s); // 12 symbols
        
        //backoff method to use: constant, linear or exponential
        string backoffMethod = default("linear");
        // maximum backoffs
        double macMaxCSMABackoffs = default(5);
        // base unit for all backoff calculations
        double aUnitBackoffPeriod @unit(s) = default(0.00032 s);
        // # of backoff periods of the initial contention window
        // (for linear and constant backoff method only)
        int contentionWindow = default(2);
        // minimum backoff exponent (for exponential backoff method only)
        double macMinBE = default(3);
        // maximum backoff exponent (for exponential backoff method only)
        double macMaxBE = default(8);

}

