package org.mixim.base.modules;

// Module which is responsible for mobility
// related information like position and movement
// BaseMobility itself defines a static mobility
// pattern (means only a position, no movement).
// Mobility modules which extend from this module
// will define more complex movement patterns.
simple BaseMobility like IMobility
{
    parameters:
        bool notAffectedByHostState = default(true);
        bool coreDebug = default(false); // debug switch for the core framework
        double x = default(-1); // x coordinate of the nodes' position (-1 = random)
        double y = default(-1); // y coordinate of the nodes' position (-1 = random)
        double z = default(0); // z coordinate of the nodes' position (-1 = random)
        @display("i=block/cogwheel");
}

