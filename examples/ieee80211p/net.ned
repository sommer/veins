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

package org.mixim.examples.ieee80211p;

import org.mixim.base.modules.BaseNetwork;

//
// TODO auto-generated module
//
network net extends BaseNetwork
{
    parameters:
        int numHosts;

    submodules:
        host[numHosts]: Host80211p {
            @display("p=137,95");
        }

    connections allowunconnected:
    // all connections and gates are to be generated dynamically
}
