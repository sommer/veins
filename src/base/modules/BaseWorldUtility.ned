package org.mixim.base.modules;

// Basic utility module for the whole network.
// Provides utility methods and information used by
// the whole network as well as simulation wide
// black board functionality.
simple BaseWorldUtility extends Blackboard
{
    parameters:
        @class(BaseWorldUtility);
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        bool useTorus;                  // use the playground as torus?
        bool use2D;                    // use a 2-dimensional world?
        @display("i=misc/globe");
}

