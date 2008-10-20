package org.mixim.base.localization;

// The localization Layer for BaseLocalization.
//
// This module has to be extended in order to implement a 
// localization algorithm.
//
// @author Peterpaul Klein Haneveld
moduleinterface BaseLocalization
{
    parameters:
        bool debug; // debug switch
        bool isAnchor;
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

