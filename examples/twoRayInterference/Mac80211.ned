//***************************************************************************
// * file:        Network.ned
// *
// * author:      Marc Loebbers
// *
// * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
// *              Technische Universitaet Berlin, Germany.
// *
// *              This program is free software; you can redistribute it 
// *              and/or modify it under the terms of the GNU General Public 
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later 
// *              version.
// *              For further information see file COPYING 
// *              in the top level directory
// ***************************************************************************
// * part of:     framework implementation developed by tkn
// * description: a network to test the 802.11 modules
// *              
// ***************************************************************************
// * changelog:   $Revision: 103 $
// *              last modified:   $Date: 2004-09-23 09:56:00 +0200 (Do, 23 Sep 2004) $
// *              by:              $Author: omfw-willkomm $
// **************************************************************************/
package org.mixim.examples.twoRayInterference;

import org.mixim.base.modules.BaseNetwork;
import org.mixim.modules.node.Host80211;

network Mac80211 extends BaseNetwork
{
    parameters:
        int numHosts; // total number of hosts in the network  

    submodules:
        host[numHosts]: Host80211 {
            @display("p=137,95");
        }

    connections allowunconnected:
    // all connections and gates are to be generated dynamically
}
