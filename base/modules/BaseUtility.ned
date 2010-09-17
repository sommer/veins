package org.mixim.base.modules;

// Basic utility module for a host.
// Mainly provides black board like information
// exchange between the other modules of a host.
simple BaseUtility extends Blackboard
{
    parameters:
        @class(BaseUtility);
        @display("i=block/blackboard");
}

