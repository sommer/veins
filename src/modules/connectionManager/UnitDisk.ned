// - mode:c++ -//- //////////////////////////////////////////////////////////
// file:        UnitDisk.ned
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
/////////////////////////////////////////////////////////////////////////////
// description: propagation layer: unit disk model
////////////////////////////////////////////////////////////////////////////

package org.mixim.modules.connectionManager;

simple UnitDisk
{
    parameters:
        bool   debug = default(false); // debug switch
        double radioRange;

        // below are unused here, but needed for BasePhyLayer
        // maximum sending power used for this network (in mW)
        double pMax; // maximum sending power used for this network (in mW)
        // minimum signal attenuation threshold (in dBm)
        double sat; // minimum signal attenuation threshold (in dBm)
        // minimum path loss coefficient
        double alpha; // minimum path loss coefficient
        // minimum carrier frequency of the channel (in Hz)
        double carrierFrequency; // minimum carrier frequency of the channel (in Hz)
}

