package org.mixim.modules.localization;

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
        input upperGateIn; // from application layer
        output upperGateOut; // to application layer
        input upperControlIn; // control from application layer
        output upperControlOut; // control to application layer
        input lowerGateIn; // from network layer
        output lowerGateOut; // to network layer
        input lowerControlIn; // control from network layer
        output lowerControlOut; // control to network layer
}

