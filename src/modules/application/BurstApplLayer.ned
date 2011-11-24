//****************************************************************************
// * file:        BurstApplLayer.ned
// *
// * author:      Daniel Willkomm
// *
// * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
// *              Technische Universitaet Berlin, Germany.
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
// * description: application layer like the TestApplLayer but sends burst of 
// *               messages
// *
// ***************************************************************************
// * changelog:   $Revision$
// *              last modified:   $Date: 2004-09-28 00:46:06 +0200 (Tue, 28 Sep 2004) $
// *              by:              $Author: omfw-willkomm $
// ***************************************************************************/
package org.mixim.modules.application;

import org.mixim.base.modules.IBaseApplLayer;

// Application layer to test lower layer implementations
//
// This application layer does exactly the same as the
// TestApplLayer. The only difference is that is sends a burst of
// broadcast messages instead of just one. The burst size is specified
// in burstSize and can be set in omnetpp.ini
//
// @see TestApplLayer
// @author Daniel Willkomm
simple BurstApplLayer like IBaseApplLayer
{
    parameters:
        bool coreDebug  = default(false); //debug switch for base class (TestApplLayer)
    	bool debug      = default(false); // debug switch
    	int  headerLength @unit("bit");   // length of the application message header (in bits)
    	int  burstSize;                   // size of the burst
    	bool burstReply = default(true);  // if true, send unicast replies to received broadcasts
    gates:
    	input lowerLayerIn; // from network layer
        output lowerLayerOut; // to network layer
        input lowerControlIn; // control from network layer
        output lowerControlOut; // control to network layer
}