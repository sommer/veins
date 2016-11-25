package org.car2x.veins.base.modules;

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
        double x; // x coordinate of the nodes' position (-1 = random)
        double y; // y coordinate of the nodes' position (-1 = random)
        double z; // z coordinate of the nodes' position (-1 = random)
        double xOrientation = default(1);
        double yOrientation = default(0);
        double zOrientation = default(0);
        @signal[veinsmobilityStateChanged](type="BaseMobility");
        @display("i=block/cogwheel");
}

