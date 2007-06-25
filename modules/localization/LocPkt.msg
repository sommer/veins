//**************************************************************************
// * file:        LocPkt.msg
// *
// * author:      Peterpaul Klein Haneveld
// *
// * copyright:   (C) 2007 Parallel and Distributed Systems Group (PDS) at
// *              Technische Universiteit Delft, The Netherlands.
// *
// *              This program is free software; you can redistribute it 
// *              and/or modify it under the terms of the GNU General Public 
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later 
// *              version.
// *              For further information see file COPYING 
// *              in the top level directory
// **************************************************************************
// * description: - base class for localization messages
// *              - subclass to create your own localization messages
// **************************************************************************/


// A basic localization message definition
//
// subclass if you want to create your own localization message class
//
// @author Peterpaul Klein Haneveld
message LocPkt {
    fields:
	int type;
	int destAddr;
	int srcAddr;
	char data[];
}