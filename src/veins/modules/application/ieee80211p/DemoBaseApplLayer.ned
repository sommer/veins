//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

package veins.modules.application.ieee80211p;
import veins.base.modules.IBaseApplLayer;

//
// Demo application layer base class.
//
// @author David Eckhoff
//
// @see DemoBaseApplLayer
// @see Nic80211p
// @see Mac1609_4
// @see PhyLayer80211p
//
simple DemoBaseApplLayer like IBaseApplLayer
{
    parameters:
        @class(veins::DemoBaseApplLayer);
        int headerLength = default(88bit) @unit(bit);

        bool sendBeacons = default(true);
        int beaconLengthBits = default(256bit) @unit(bit);
        int beaconUserPriority = default(7);
        double beaconInterval = default(1s) @unit(s);

        int dataLengthBits = default(1024bit) @unit(bit);
        bool dataOnSch = default(false);
        int dataUserPriority = default(7);

        int denmLengthBits = default(512bit) @unit(bit);
        int denmUserPriority = default(7);
        int denmDefaultCause = default(-1);
        int denmDefaultSubcause = default(0);
        double denmStartTime @unit(s) = default(0s);
        double denmStopTime  @unit(s) = default(200s);
        double denmInterval  @unit(s) = default(1s);

        bool avoidBeaconSynchronization = default(true);

        bool sendWSA = default(false);
        int wsaLengthBits = default(250bit) @unit(bit);
        double wsaInterval =  default(1s) @unit(s);

    gates:
        input lowerLayerIn;
        output lowerLayerOut;
        input lowerControlIn;
        output lowerControlOut;
}
