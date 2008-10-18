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
// * description: - base class for localization packets
// *              - subclass to create your own localization packets
// **************************************************************************/

cplusplus {{
#include "Location.h"
}}



class noncobject Location;

// A basic localization packet definition
//
// subclass if you want to create your own localization packet class
//
// @author Peterpaul Klein Haneveld
packet LocPkt
{
    int id;
    bool isAnchor;
    Location pos;
}
