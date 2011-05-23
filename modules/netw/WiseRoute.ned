package org.mixim.modules.netw;
//***************************************************************************
// file:        WiseRoute.ned
//
// author:      Damien Piguet, Jerome Rousselot
// 
// copyright:   (C) 2007-2009 CSEM SA, Neuchatel, Switzerland.
//
// description: Implementation of the routing protocol of WiseStack.
//
// *              This program is free software; you can redistribute it 
// *              and/or modify it under the terms of the GNU General Public 
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later 
// *              version.
// *              For further information see file COPYING 
// *              in the top level directory
// *
// * Funding: This work was partially financed by the European Commission under the  
// * Framework 6 IST Project "Wirelessly Accessible Sensor Populations"  
// * (WASP) under contract IST-034963.
// ***************************************************************************
// * ported to Mixim 2.0.1 by Theodoros Kapourniotis
// * last modification: 06/02/11
// **************************************************************************/
import org.mixim.base.modules.BaseNetwLayer;

simple WiseRoute extends BaseNetwLayer
{
    parameters:
        // debug switch
        bool debug = default(false);
        bool trace = default(false);
     	bool useSimTracer = default(false);
     	
        // sink node IP address (integer)
        int sinkAddress = default(0);
        // the sink directs the tree building procedure with periodic floods.
        // iterationDelay is the period between two floods.
        
        
        // RSSI threshold for route selection
        double rssiThreshold @unit(dBm) = default(-50 dBm);
        
        // If set to zero, this node does not initiates route tree building.
        // If set to a value larger than zero, this nodes periodically initiates route tree building.
        double routeFloodsInterval @unit(s) = default(0 s);
        @display("i=block/fork");
        @class(WiseRoute);

//   gates:
//        input uppergateIn; // from application layer
//        output uppergateOut; // to application layer
//        output upperControlOut; // for control messages to upper layer
//        input lowerControlIn;   // for control messages from mac layer
//        input lowergateIn; // from MAC layer
//        output lowergateOut; // to MAC layer
}

