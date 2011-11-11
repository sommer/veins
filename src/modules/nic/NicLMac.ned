//
//  LMAC for MF 2.02, omnetpp 3.4
//
//  Created by Anna Foerster on 10/10/08.
//  Copyright 2008 Universita della Svizzera Italiana. All rights reserved.
//
//  Coverted to OMNeT++ 4 by Rudolf Hornig
//
//  Converted to MiXiM by Kapourniotis Theodoros
//
//

package org.mixim.modules.nic;

//
// This module implements a LMAC network interface card using
// the "LMacLayer" MAC layer module
//
module NicLMAC extends WirelessNicBattery
{
    parameters:
        macType = default("LMacLayer");
}
