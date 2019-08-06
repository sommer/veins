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
        @class(veins::BaseMobility);
        bool notAffectedByHostState = default(true);
        double x; // x coordinate of the nodes' position (-1 = random)
        double y; // y coordinate of the nodes' position (-1 = random)
        double z; // z coordinate of the nodes' position (-1 = random)
        @signal[org_car2x_veins_base_modules_mobilityStateChanged](type="veins::BaseMobility");
        @display("i=block/cogwheel");
}

