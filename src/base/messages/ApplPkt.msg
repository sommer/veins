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
// * description: - base class for application layer packets
// *              - subclass to create your own application layer packets
// **************************************************************************/

cplusplus {{
    #include "SimpleAddress.h"
}}

class LAddress::L3Type extends void;

// Basic application packet definition
//
// subclass if you want to create your own application layer packet
// class
//
// @author Daniel Willkomm
packet ApplPkt
{
    LAddress::L3Type destAddr = LAddress::L3BROADCAST; // destination address
    LAddress::L3Type srcAddr  = LAddress::L3BROADCAST; // source address
}
