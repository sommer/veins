//**************************************************************************
// * file:        ApplPkt.msg
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
// * description: - base class for application layer messages
// *              - subclass to create your own application layer messages
// **************************************************************************/


// Basic application message definition
//
// subclass if you want to create your own application layer message
// class
//
// @author Daniel Willkomm
packet ApplPkt
{
    int destAddr = -1; // destination address
    int srcAddr = -1; // source address

}
