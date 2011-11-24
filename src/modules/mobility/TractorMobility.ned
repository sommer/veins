//
// Copyright (C) 2007 Peterpaul Klein Haneveld
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package org.mixim.modules.mobility;

import org.mixim.base.modules.BaseMobility;

//
// Moves a tractor through a field with a certain amount of rows.
// Since the tractor also moves around the field, the tractor travels
// the number of rows PLUS one rows. Consider the following piece of
// ascii-art for rows=2.
//
// >>>>>>>>>>>>>>>v
// ^              v
// ^              v
// <<<<<<<<<<<<<<<<
// v              ^
// v              ^
// >>>>>>>>>>>>>>>^
//
// NOTE: Does not yet support 3-dimensional movement.
// @author Peterpaul Klein Haneveld
//
simple TractorMobility extends BaseMobility
{
    parameters:
        @class(TractorMobility);
        bool debug = default(false); // debug switch
        double x1 @unit(m); // rectangle is denoted by the (x1,y1) and (x2,y2) coords [m]
        double y1 @unit(m); // rectangle is denoted by the (x1,y1) and (x2,y2) coords [m]
        double x2 @unit(m); // rectangle is denoted by the (x1,y1) and (x2,y2) coords [m]
        double y2 @unit(m); // rectangle is denoted by the (x1,y1) and (x2,y2) coords [m]
        double rows; // the number of rows the field is divided in
        double speed @unit(mps); // speed of the host (in m/s)
        double updateInterval @unit(s); // time interval to update the hosts position (in seconds)
}

