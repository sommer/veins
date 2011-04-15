package org.mixim.modules.netw;

import org.mixim.base.modules.BaseArp;

// Module which is responsible for Addressresolution
simple ArpHost extends BaseArp
{
    parameters:
        @class(ArpHost);        
        bool debug = default(false); 	// enable debugging for this module
		int offset = default(0);  		// Adds an offset to node addresses
}

