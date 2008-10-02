// The localization Layer for BaseLocalization.
//
// This module has to be extended in order to implement a 
// localization algorithm.
//
// @author Peterpaul Klein Haneveld
simple BaseLocalization
{
    parameters:
        bool debug; // debug switch
        bool isAnchor;
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

