///*
// * author:      Jerome Rousselot
// *
// * copyright:   (C) 2010 CSEM SA, Neuchatel, Switzerland.
// *
// * description: ExtendedNetwLayer allows to choose between ARP layers.
// *				By default, it selects ArpHost, which uses the host module index
// *				for both the mac and net addresses.
// *
// *              This program is free software; you can redistribute it
// *              and/or modify it under the terms of the GNU General Public
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later
// *              version.
// *              For further information see file COPYING
// *              in the top level directory
// *
// */

package org.mixim.modules.netw;

import org.mixim.base.modules.BaseNetwLayer;

// Network layer module that allows to choose the ARP module
simple ExtendedNetwLayer extends BaseNetwLayer
{
    parameters:
        @class(ExtendedNetwLayer);
		string arpMode = default("Host");      // Can be Host (uses Host index for both mac and net addresses) or Auto (uses module ID)  
}

