package org.mixim.modules.transport;
//***************************************************************************
// file:        Aggregation.ned
//
// author:      Jerome Rousselot <jerome.rousselot@csem.ch>
// 
// copyright:   (C) 2010 CSEM SA, Neuchatel, Switzerland.
//
// description:  this class aggregates the packets received from the application
// 				 layer and separates packet emissions by a time InterPacketDelay.
//
// *              This program is free software; you can redistribute it 
// *              and/or modify it under the terms of the GNU General Public 
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later 
// *              version.
// *              For further information see file COPYING 
// *              in the top level directory
// *
// **************************************************************************/

import org.mixim.base.modules.BaseLayer;

simple Aggregation extends BaseLayer
{
    parameters:
        // debug switch
        bool trace = default(false);
        bool stats = default(true);
        int headerLength @unit(byte) = default(2 byte);
        double interPacketDelay @unit(s) = default(1 s); // this class does not send more than two packets to the same destination in a time interPacketDelay to the lower layer.
        int nbMaxPacketsPerAggregation = default(10); // maximum number of packets to aggregate per sending
        @class(Aggregation);
}

