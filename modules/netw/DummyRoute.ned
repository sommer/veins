package org.mixim.modules.netw;
//***************************************************************************
// file:        DummyRoute.ned
//
// author:      Jerome Rousselot
// 
// copyright:   (C) 2009 CSEM SA, Neuchatel, Switzerland.
//
// description:  Adaptor module that simply "translates" netwControlInfo to macControlInfo
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

simple DummyRoute
{
    parameters:
        // debug switch
        bool debug = default(false);
        bool trace = default(false);
        bool stats = default(true);
        
        // lenght of the routing packet header
        double headerLength @unit(byte) = default(0 byte);

    gates:
        input upperGateIn; // from application layer
        output upperGateOut; // to application layer
        input lowerGateIn; // from MAC layer
        output lowerGateOut; // to MAC layer
        input upperControlIn;   // for control messages from mac layer
        output upperControlOut; // for control messages to upper layer
        input lowerControlIn;   // for control messages from mac layer
        output lowerControlOut; // for control messages to mac layer
        
        
}

