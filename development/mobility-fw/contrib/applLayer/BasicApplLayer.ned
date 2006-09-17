//***************************************************************************
//* file:        BasicApplLayer.ned
//*
//* author:      Daniel Willkomm
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
//* description: application layer: general class for the application layer
//*              subclass to create your own application layer
//***************************************************************************
//* changelog:   $Revision: 1.3 $
//*              last modified:   $Date: 2004/09/27 22:31:58 $
//*              by:              $Author: omfw-willkomm $
//***************************************************************************/


// Basic class for the application layer
// 
// This is the generic class for all application layer modules. If you
// want to implement your own application layer you have to subclass your
// module from this class.
//
// In this implementation all nodes randomly send broadcast packets to
// all connected neighbors. Every node who receives this packet will
// send a reply to the originator node.
//
// @author Daniel Willkomm
simple BasicApplLayer
    parameters:
	debug : bool, // debug switch
	headerLength : numeric const; // length of the application message header (in bits)
    gates:
	in: lowergateIn,  // from network layer
	    lowerControlIn;
	out: lowergateOut; // to network layer
endsimple
