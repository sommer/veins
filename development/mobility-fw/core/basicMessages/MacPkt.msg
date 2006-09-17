//**************************************************************************
// * file:        MacPkt.msg
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
// **************************************************************************
// * part of:     framework implementation developed by tkn
// * description: - base class for MAC layer messages
// *              - subclass to create your own MAC layer messages
// **************************************************************************/


// A basic MAC (Media Access Control) packet format definition
// 
// subclass if you want to create your own MAC layer message class
//
// The basic MAC message only provides source and destination address
//
// @author Daniel Willkomm
message MacPkt{
    fields:
	int destAddr; // destination mac address
	int srcAddr;  // source mac address
}
