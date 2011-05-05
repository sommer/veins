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

import org.mixim.base.modules.BaseMacLayer;

//
// CSMA MAC protocol
//
simple CSMAMacLayer extends BaseMacLayer
{
    parameters:
        @class(CSMAMacLayer);
        // enable debugging for this module
        bool debug = default(false);
        // how long is a slot? [s]
        double slotDuration @unit(s);
        // maximum time between a packet and its ack [s]
        double difs @unit(s);
        // maximum number of transmission attempts
        double maxTxAttempts;
        // length of the MAC queue
        int queueLength;
        // default channel 
        double defaultChannel;
        // bit rate [bps]
        double bitrate @unit(bps);
        //tx power [mW]
        double txPower @unit(mW);
        // contention window
        double contentionWindow;
}
