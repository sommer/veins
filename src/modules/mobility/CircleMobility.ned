//
// Copyright (C) 2005 Andras Varga
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
// Moves the node around a circle.
//
// @see ConnectionManager
// @author Andras Varga
//
simple CircleMobility extends BaseMobility
{
    parameters:
        @class(CircleMobility);
        bool debug = default(false); // debug switch
        double cx; // x coord of the center of the circle
        double cy; // y coord of the center of the circle
        double cz = default(0); // z coord of the center of the circle
        double r; // radius of the circle
        double speed @unit(mps); // speed of the host (in m/s)
        double startAngle @unit(deg); // starting angle (degreees)
        double updateInterval @unit(s); // time interval to update the hosts position (seconds)
}

