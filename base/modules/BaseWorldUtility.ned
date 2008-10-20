package org.mixim.base.modules;

simple BaseWorldUtility
{
    parameters:
        bool coreDebug;       // debug switch for the base framework
        double playgroundSizeX; // x size of the area the nodes are in (in meters)
        double playgroundSizeY; // y size of the area the nodes are in (in meters)
        double playgroundSizeZ; // z size of the area the nodes are in (in meters)
        bool useTorus;                  // use the playground as torus?
        bool use2D;                    // use a 2-dimensional world?
}

