//***************************************************************************
// * file:        BasicMobility.ned
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
// * description: Basic module for all mobility modules
// **************************************************************************/


// Basic module for all mobility modules.
//
// The basic module mainly provides the interfaces with the
// ChannelControl module and functions to update the graphical user
// interface.
//
// It does not provide mobility at all, so you can use it if you only
// want to simulate static networks.
//
// BasicMobility provides random placement of hosts and display
// updates as well as registering with the ChannelControl module. 
//
// Additionally complete border handling is implemented in BasicMobility.
//
// @see ChannelControl
// @author Daniel Willkomm
simple BasicMobility
    parameters:
	coreDebug : numeric const, // debug switch for the core framework
        x: numeric const, // x coordinate of the nodes' position (-1 = random)
        y: numeric const; // y coordinate of the nodes' position (-1 = random)
endsimple
