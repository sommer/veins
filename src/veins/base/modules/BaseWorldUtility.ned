package org.car2x.veins.base.modules;

// Basic utility module for the whole network.
// Provides utility methods and information used by
// the whole network as well as simulation wide
// black board functionality.
simple BaseWorldUtility
{
    parameters:
        @class(Veins::BaseWorldUtility);
        double playgroundSizeX @unit(m);    // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m);    // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m);    // z size of the area the nodes are in (in meters)
        bool   useTorus = default(false);   // use the playground as torus?
        bool   use2D    = default(false);   // use a 2-dimensional world?
        @display("i=misc/globe");
}

