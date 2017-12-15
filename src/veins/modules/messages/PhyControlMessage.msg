//
// Copyright (C) 2014 Michele Segata <segata@ccs-labs.org>
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

//
// Defines a control message that can be associated with a MAC frame to set
// transmission power and datarate on a per packet basis
//
message PhyControlMessage {
    //modulation and coding scheme to be used (see enum TxMCS in ConstsPhy.h)
    int mcs = -1;
    //transmission power to be used in mW
    double txPower_mW = -1;
}
