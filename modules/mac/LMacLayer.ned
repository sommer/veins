//
//  LMACLayer.cc
//  BMAC for MF 2.02, omnetpp 4
//
//  Created by Anna Foerster on 10/10/08.
//  Copyright 2008 Universita della Svizzera Italiana. All rights reserved.
//
//  Coverted to OMNeT++ 4 by Rudolf Hornig
//
//  Converted to MiXiM by Kapourniotis Theodoros
//

package org.mixim.modules.mac;

import org.mixim.base.modules.BaseMacLayer;

simple LMacLayer extends BaseMacLayer
{
    parameters:
        // how long is a slot?
        double slotDuration @unit(s) = default(100ms);
        // length of MAC header
//        double headerLength @unit(byte) = default(10byte);
        // length of the MAC queue
        double queueLength = default(10);
        // default channel 
        double defaultChannel = default(0);
        // bit rate 
        double bitrate @unit(bps) = default(19200bps);
        // level at which we consider medium busy FIXME not used currently
//        double busyRSSI;
        
        int reservedMobileSlots = default(2);
        int numSlots = default(64);
        double txPower = default(50);
        
        @class(LMacLayer);

//    gates:
//        input uppergateIn;
//        output uppergateOut;
//        input lowergateIn;
//        output lowergateOut;
//        output upperControlOut; // for control messages to upper layer
//        input lowerControlIn;   // for control messages from phy layer
}
