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


package org.mixim.modules.mac;

//
// CSMA MAC protocol
//
simple CSMAMacLayer
{
    parameters:
        // level at which we consider medium busy [dB]
        double busyRSSI;
        // how long is a slot?
        double slotDuration;
        // maximum time between a packet and its ack
        double difs;
        // maximum number of transmission attempts
        double maxTxAttempts;
        // length of the MAC queue
        int queueLength;
        // default channel 
        double defaultChannel;
        // bit rate 
        double bitrate @unit(bps);
        //tx power [mW]
        double txPower;
        // contention window
        double contentionWindow;


        // debug switch
        // debug switch
        bool coreDebug; // debug switch
        double headerLength; // length of the MAC packet header (in bits)

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
