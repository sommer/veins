// The localization Layer for BaseLocalization.
//
// This module has to be extended in order to implement a 
// localization algorithm.
//
// @author Peterpaul Klein Haneveld
simple BaseLocalization
    parameters:
        debug: numeric const, // debug switch
        headerLength: numeric const; // length of the network packet header (in bits)
    gates:
        in: uppergateIn; // from application layer
        out: uppergateOut; // to application layer
        in: upperControlIn; // control from application layer
        out: upperControlOut; // control to application layer
        in: lowergateIn; // from network layer
        out: lowergateOut; // to network layer
        in: lowerControlIn; // control from network layer
        out: lowerControlOut; // control to network layer
endsimple

