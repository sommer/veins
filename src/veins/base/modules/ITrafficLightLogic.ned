//
// Copyright (C) 2015 Dominik Buse <dbuse@mail.uni-paderborn.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package org.car2x.veins.base.modules;

// Interface for signal controller logic of traffic lights
moduleinterface ITrafficLightLogic
{
    parameters:
        @display("i=block/classifier");
   	gates:
		inout interface;
		inout applLayer;
		//Not need to be connected, as a traffic light might not have an induction loop
		input inductionLoop[] @loose; // gates for trigger signals from induction loops
}
