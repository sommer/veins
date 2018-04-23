//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

package org.car2x.veins.modules.application.unicast;
import org.car2x.veins.base.modules.IBaseApplLayer;

//
// Sample unicast application 
//
// @author David Eckhoff
// @author Gurjashan Pannu
//
// @see UnicastApplLayer
// @see Nic80211p
// @see Mac1609_4
// @see PhyLayer80211p
//
simple UnicastApplLayer like IBaseApplLayer
{
    parameters:
        bool debug = default(false); //whether to print debug output
        int headerLength = default(88bit) @unit(bit); //header length of the application

        double maxOffset = default(0s) @unit(s); //the maximum delay of messages sent down to avoid synchronous channel access
        
        // unicast 
        bool sendUnicast = default(false); // whether to send unicast packets or not
        volatile double unicastInterval = default(1s) @unit(s); // the interval between 2 unicast messages
        int unicastLengthBits = default(256bit) @unit(bit); // the length of unicast packet
        int unicastPriority = default(1); // access category for unicast packets. 4 means random
        double unicastStartTime @unit(s) = default(0s); // tell the applayer to start sending unicast from this time onwards
        
        double periodicMeasurementStartTime @unit(s) = default(0s); // when to start periodic measurement
        double periodicMeasurementInterval @unit(s) = default(1s); // time gap between two consecutive periodic measurements

        @signal[sigChannelLoad](type="double");
        @statistic[channelLoad](title="ChannelLoad"; source="sigChannelLoad"; record=vector;);
        
        @signal[sigGoodput](type="long");
        @statistic[goodput](title="Goodput"; source="sigGoodput"; record=vector;);

    gates:
        input lowerLayerIn; // from mac layer
        output lowerLayerOut; // to mac layer
        input lowerControlIn;
        output lowerControlOut;
        
}
