package org.car2x.veins.base.modules;

// Interface for mobility modules
moduleinterface IMobility
{
    parameters:
        double x; // x coordinate of the nodes' position (-1 = random)
        double y; // y coordinate of the nodes' position (-1 = random)
        double z; // z coordinate of the nodes' position (-1 = random)
        @display("i=block/cogwheel");
}

