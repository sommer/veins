//**************************************************************************
// * file:        BorderMsg.msg
// *
// * author:      Daniel Willkomm
// *
// * copyright:   (C) 2006 Telecommunication Networks Group (TKN) at
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

cplusplus {{
#include "veins/base/utils/Coord.h"
}}

namespace Veins;

class noncobject Coord;

// used as a self packet containing the position, direction and BorderPolicy
//
// @author Daniel Willkomm
//
packet BorderMsg
{
    int policy;    // border policy to use for the host
    Coord startPos; // new start position of the host
    Coord direction; // new direction of the host
}
