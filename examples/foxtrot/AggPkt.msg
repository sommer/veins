//// -//- mode:c++ -//- ///////////////////////////////////////////////////
// file:      AggPkt.msg
//
// author:      Tom Parker
//
// copyright:   (C) 2006 Parallel and Distributed Systems Group (PDS) at
//              Technische Universiteit Delft, The Netherlands.
//
//              This program is free software; you can redistribute it 
//              and/or modify it under the terms of the GNU General Public 
//              License as published by the Free Software Foundation; either
//              version 2 of the License, or (at your option) any later 
//              version.
//              For further information see file COPYING 
//              in the top level directory
////////////////////////////////////////////////////////////////////////////
// part of:     wsn-specific modules
// description: aggregation layer, data packet
///////////////////////////////////////////////////////////////////////////

// A basic aggregation layer message definition
//
// subclass if you want to create your own aggregation layer message class
//
// The basic aggregation packet contains a destination aggregation address.
//
// @author Tom Parker
message AggPkt
{
    int srcAddr; 		 // source addr. May be broadcast if this is built from many packets
    int destAddr;        // destination address
    double data[]; 		 // raw data from app layer
}

