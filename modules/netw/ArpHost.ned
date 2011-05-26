package org.mixim.modules.netw;

import org.mixim.base.modules.BaseArp;

// Uses host index as network and MAC address.
//
// It assumes that hosts are declared as an array of NED modules,
// and returns for both the level 2 and level 3 addresses, the
// index of the current host in this array.
simple ArpHost extends BaseArp
{
    parameters:
        @class(ArpHost);
        bool debug = default(false); 	// enable debugging for this module
		int offset = default(0);  		// Adds an offset to node addresses
}

