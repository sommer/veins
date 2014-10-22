//
//  Copyright (C) 2009 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
//  Documentation for these modules is at http://veins.car2x.org/
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

package org.car2x.veins.modules.application.traci;

import org.car2x.veins.base.modules.IBaseApplLayer;

simple TraCITestApp like IBaseApplLayer
{
	parameters:
		bool debug = default(false);  // output debugging information
		int headerLength @unit("bit") = default (0bit);  // length of the application message header (in bits)
		int testNumber;  // number of test to run (-1: no test)

		@display("i=block/app2");
		@class("Veins::TraCITestApp");
	gates:
		input lowerLayerIn; // from network layer
		output lowerLayerOut; // to network layer
		input lowerControlIn; // control from network layer
		output lowerControlOut; // control to network layer
}
