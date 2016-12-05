//
// Copyright (C) 2016 Julian Heinovski <julian.heinovski@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package org.car2x.veins.modules.mac.aribt109;

import org.car2x.veins.base.modules.IWirelessMac;

// Manages virtual and physical carrier sense function for medium access.
// Does not manage logical links
//
// @author Julian Heinovski
//
moduleinterface IAribT109Mac extends IWirelessMac
{
	parameters:

        // bit rate 
        double bitrate @unit(bps);

        // tx power [mW]
        double txPower @unit(mW);
        
        double ccaThreshold @unit(dBm);

        // signal informing interested application about channel busy state
        @signal[sigChannelBusy](type=bool);
        
        // signal informing interested application about a collision
        @signal[sigCollision](type=bool);
        
		// for more signals, see corresponding module
}
