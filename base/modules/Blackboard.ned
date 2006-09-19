//***************************************************************************
//* file:        Blackboard.ned
//*
//* author:      Andreas Koepke
//*              (reimplementation of a file by Steffen Sroka)
//*
//* copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
//*              Technische Universitaet Berlin, Germany.
//*
//*              This program is free software; you can redistribute it 
//*              and/or modify it under the terms of the GNU General Public 
//*              License as published by the Free Software Foundation; either
//*              version 2 of the License, or (at your option) any later 
//*              version.
//*              For further information see file COPYING 
//*              in the top level directory
//***************************************************************************
//* part of:     framework implementation developed by tkn
//* description: Entity to enable inter layer / process communication
//***************************************************************************
//* changelog:   $Revision: 164 $
//*              last modified:   $Date: 2005-09-26 17:29:52 +0200 (Mon, 26 Sep 2005) $
//*              by:              $Author: koepke $
//**************************************************************************/

// The Blackboard works as entity to enable inter layer /
// process communication.
//
// Blackboard makes global host knowledge or information relevant to
// more than one layer accessible to all modules derived from
// BlackboardAccess.
//
// Every module derived from BlackboardAccess is capable to publish
// information on the Blackboard as well as to subscribe for it.
//
// @author Steffen Sroka, Andreas Koepke
simple Blackboard
    parameters:
	coreDebug : numeric const; // debug switch for core framework
endsimple
