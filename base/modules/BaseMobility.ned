package org.mixim.base.modules;

// Module which is responsible for mobility
// related information like position and movement
// BaseMobility itself defines a static mobility
// pattern (means only a position, no movement).
// Mobility modules which extend from this module
// will define more complex movement patterns.
simple BaseMobility like IBaseMobility
{
    parameters:
        bool notAffectedByHostState = true;
        bool coreDebug; // debug switch for the core framework
        double x; // x coordinate of the nodes' position (-1 = random)
        double y; // y coordinate of the nodes' position (-1 = random)
        double z; // z coordinate of the nodes' position (-1 = random)
        //enable scaling of nodes depending on the z-coordinate to represent
        //depth when 3d is enabled and tkenv is used
        bool scaleNodeByDepth = default(true);
        // the following three fields represent the host orientation
        // as a normalized vector: iDX²+iDY²+iDZ²=1
        // e.g. iDX=1, iDY=iDZ=0 -> the host is pointing toward X axis (positive)
        // this is needed for stationary hosts
        double initialDirectionX = default(1);
        double initialDirectionY = default(0);
        double initialDirectionZ = default(0);
        @display("i=block/cogwheel");
}

