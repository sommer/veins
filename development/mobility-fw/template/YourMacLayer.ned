//****************************************************************************
// * file:        YourMacLayer.ned
// *
// * author:      Your Name
// *
// * copyright:   (C) 2004 Your Institution
// *
// *              This program is free software; you can redistribute it 
// *              and/or modify it under the terms of the GNU General Public 
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later 
// *              version.
// *              For further information see file COPYING 
// *              in the top level directory
// ***************************************************************************
// * part of:     Your Simulation
// * description: - Your Description
// *
// ***************************************************************************
// * changelog:   $Revision: 388 $
// *              last modified:   $Date: 2006-08-10 11:02:52 +0200 (Do, 10 Aug 2006) $
// *              by:              $Author: willkomm $
// ***************************************************************************/


simple YourMacLayer
    parameters:
        debug: bool, // debug switch
        // level at which we consider medium busy
	busyRSSI: numeric const, 
	// length of MAC header
        headerLength: numeric const,
	// length of the MAC queue
        defaultChannel : numeric const,
	// bit rate 
	bitrate : numeric const;
    gates:
        in: uppergateIn;
        out: uppergateOut;
        in: lowergateIn;
        out: lowergateOut;
	out: upperControlOut; // for control messages to upper layer
	in: lowerControlIn;   // for control messages from phy layer
endsimple
