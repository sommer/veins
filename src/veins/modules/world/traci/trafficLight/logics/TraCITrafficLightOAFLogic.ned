//
// Copyright (C) 2015 Dominik Buse <dbuse@mail.uni-paderborn.de>
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

package org.car2x.veins.modules.world.traci.trafficLight.logics;

import org.car2x.veins.base.modules.ITrafficLightLogic;

simple TraCITrafficLightOAFLogic like ITrafficLightLogic {
    parameters:
        @class(Veins::TraCITrafficLightOAFLogic);
        @display("i=block/classifier");

        xml cgraph;
        string customLogPath = default("result/dummy.file");

        double maxGreenTime @unit(s);
        double yellowTime @unit(s);
        double redTime @unit(s);

        double approachingSpeed @unit(mps) = default(3.0mps);
        double junctionProximityThreshold @unit(m) = default(5.0m);
        double vehicleRecordTimeout @unit(s) = default(5.0s);
        double scoreGainPerVehicle = default(0.2);
        double maxDistance = default(300.0);

   	gates:
		inout interface;
		inout applLayer;
		input inductionLoop[] @loose;
}
