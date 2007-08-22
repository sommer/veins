//***************************************************************************
// * file:        TestConnectionManager.ned
// *
// * author:      Steffen Sroka, Daniel Willkomm
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
// **************************************************************************/


// Module to control all connection related stuff
//
// The central module that coordinates the connections between all
// nodes, and handles dynamic gate creation.  ConnectionManager therefore
// periodically communicates with the mobility module and
// ChannelAccess.
//
// The four parameters pMax, sat, alpha, and carrierFrequency are used
// to calculate the interference distance between nodes. The values used
// here in ConnectionManager are used to calculate the upper bound,
// i.e. they can be redifined in the snrEval modules, but never such
// that the maximal interference distance is exeeded.
//       
// @author Steffen Sroka, Daniel Willkomm
// @see BasicMobility
//
simple TestConnectionManager
    parameters:
	coreDebug		  : numeric const, // debug switch for core framework
	sendDirect                : bool,          // define which ConnectionManager module to use
	pMax			  : numeric const, // maximum sending power used for this network (in mW)
	sat			  : numeric const, // minimum signal attenuation threshold (in dBm)
	alpha			  : numeric const, // minimum path loss coefficient
	carrierFrequency	  : numeric const; // minimum carrier frequency of the channel (in Hz)
endsimple

