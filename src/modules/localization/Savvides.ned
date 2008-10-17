// The localization Layer for Savvides.
//
// This module has to be extended in order to implement a 
// localization algorithm.
//
// @author Peterpaul Klein Haneveld
simple Savvides
{
    parameters:
        bool debug; // debug switch
        bool isAnchor;
        double refine_limit;
        double flood_limit;
        double repeats;
        double tri_alg;
        double accuracy;
        double phase1_min_anchors;
        double phase1_max_anchors;
        double headerLength; // length of the network packet header (in bits)
    gates:
        input uppergateIn; // from application layer
        output uppergateOut; // to application layer
        input upperControlIn; // control from application layer
        output upperControlOut; // control to application layer
        input lowergateIn; // from network layer
        output lowergateOut; // to network layer
        input lowerControlIn; // control from network layer
        output lowerControlOut; // control to network layer
}

