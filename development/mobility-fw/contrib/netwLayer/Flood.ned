//***************************************************************************
// * file:        Flood.ned
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
// * description: a simple flooding protocol
// *              the user can decide whether to use plain flooding or not
// ***************************************************************************
// * changelog:   $Revision: 283 $
// *              last modified:   $Date: 2006-05-26 07:59:19 +0200 (Fr, 26 Mai 2006) $
// *              by:              $Author: koepke $
// **************************************************************************/


//
// A simple flooding protocol
//
// This implementation uses plain flooding, i.e. it "remembers"
// (stores) already broadcasted messages in a list and does not
// rebroadcast them again, if it gets another copy of that message.
//
// The maximum number of entires for that list can be defined in the
// .ini file (@ref bcMaxEntries) as well as the time after which an entry
// is deleted (@ref bcDelTime).
//
// If you prefere a memory-less version you can comment out the 
// <verbatim> #define PLAINFLOODING </verbatim>
//
// @author Daniel Willkomm
//
simple Flood
    parameters:
	debug : bool, // debug switch
	headerLength : numeric const, // length of the network header
	plainFlooding: bool, // flag whether to use plain flooding
	// Max number of entries in the list of already broadcasted messages
	bcMaxEntries : numeric const,
	// Time after which an entry for an already broadcasted msg can be deleted
	bcDelTime    : numeric const,
	defaultTtl   : numeric const; // Default time-to-live (ttl) used for this module
    gates:
	in: uppergateIn; // from application layer
	out: uppergateOut; // to application layer
	out: upperControlOut; // for control messages to upper layer
	in: lowergateIn; // from MAC layer
	in: lowerControlIn;   // for control messages from mac layer
	out: lowergateOut; // to MAC layer
endsimple
