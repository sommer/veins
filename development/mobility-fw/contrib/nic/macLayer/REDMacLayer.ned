//***************************************************************************
//* file:        REDMacLayer.ned
//*
//* author:      Jochen Adamek
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
//* description: REDMacLayer
//**************************************************************************/

simple REDMacLayer
    parameters:
        debug: bool, // debug switch
        // level at which we consider medium busy
	    busyRSSI: numeric const,
	    //size of the header
	    headerLength: numeric const;

    gates:
        in: uppergateIn;
        out: uppergateOut;
        in: lowergateIn;
        out: lowergateOut;
	out: upperControlOut; // for control messages to upper layer
	in: lowerControlIn;   // for control messages from phy layer
endsimple