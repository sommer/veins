//***************************************************************************
// * file:        P2PPhyLayer.ned
// *
// * author:      Marc Loebbers, Daniel Willkomm
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
// * description: a very simple physical layer implementation
// ***************************************************************************/


// A very simple physical layer
//
// This is the simplest physical layer you can think of.
//
// It decides upon one simple parameter pBit (bit error rate)
// about the correctness of a frame. Collisions, bit errors,
// interference and anything else you can think of are combined in
// this single parameter.
//
// @author Marc Loebbers, Daniel Willkomm
simple P2PPhyLayer

    parameters:
	coreDebug : numeric const, // debug switch for the core framework
	pBit: numeric const, // bit error rate
	bitrate: numeric const, // bitrate (in bits/sec)
	transmitterPower: numeric const, // power used to transmit messages (in mW)
	headerLength: numeric const; // length  (in bits) of the AirFrame header

    gates:
        in: uppergateIn; // from MAC layer
	out: upperControlOut; // output control information
	out: uppergateOut; // to MAC layer
	in: radioIn; // gate for sendDirect

endsimple
