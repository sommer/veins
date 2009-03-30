package org.mixim.modules.application;
//****************************************************************************
// * file:        SensorApplLayer.ned
// *
// * author:      Amre El-Hoiydi, Jerome Rousselot
// *
// * copyright:   (C) 2007-2009 CSEM
// *
// *              This program is free software; you can redistribute it 
// *              and/or modify it under the terms of the GNU General Public 
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later 
// *              version.
// *              For further information see file COPYING 
// *              in the top level directory
// ***************************************************************************
// * part of:     framework implementation developed by tkn
// * description: Generate periodic traffic addressed to a sink
// *
// ***************************************************************************/


// Application layer to test lower layer implementations. Generate periodic traffic addressed to a sink.
//
// @sa SensorApplLayer
// @author Amre El-Hoiydi, Jerome Rousselot
simple SensorApplLayer
{
    parameters:
        bool debug; // debug switch
        bool doStats;   // stats generation switch
        double nbPackets;
        string trafficType;
        double trafficParam;
        double initializationTime; // minimum time before generation of the first packet
        double headerLength @unit(byte); // length of the application payload

    gates:
        input lowerGateIn; // from network layer
        input lowerControlIn;
        output lowerGateOut; // to network layer
        output lowerControlOut;
}
