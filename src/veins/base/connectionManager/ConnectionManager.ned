//***************************************************************************
// * file:        ConnectionManager.ned
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


package org.car2x.veins.base.connectionManager;

// Module to control all connection related stuff
//
// The central module that coordinates the connections between all
// nodes, and handles dynamic gate creation.  ConnectionManager therefore
// periodically communicates with the mobility module and
// ChannelAccess.
//
// The value maxInterfDist used here in ConnectionManager defines the upper bound of any transmission,
// i.e. it can be redifined in the analogue models, but never such
// that the maximal interference distance is exeeded.
//       
// @author Steffen Sroka, Daniel Willkomm, Karl Wessel, Alexander Brummer, Christoph Sommer
// @see MobilityBase
//
simple ConnectionManager
{
    parameters:
        // debug switch for core framework
        bool coreDebug;
        // send directly to the node or create separate gates for every connection
        bool sendDirect;
        // maximum interference distance [m]
        double maxInterfDist @unit(m);
        
        // should the maximum interference distance be displayed for each node?
        bool drawMaxIntfDist = default(false);
        
        @display("i=abstract/multicast");
}

