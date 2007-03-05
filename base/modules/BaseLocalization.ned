// The localization module for the BaseLocApplLayer.
//
// This module has to be extended in order to implement a certain localization
// algorithm.
//
// @author Peterpaul Klein Haneveld

// changes PKH: Thu Feb  8 09:26:10 CET 2007
// - added switch to specify a node as an anchor node

simple BaseLocalization
    parameters:
        isAnchor: bool,
        debug: bool, // debug switch
        headerLength: numeric const; // length of the application message header (in bits)
    gates:
        in: lowergateIn; // from network layer
        out: lowergateOut; // to network layer
        in: lowerControlIn; // control from network layer
        out: lowerControlOut; // control to network layer
        in: applgateIn; // from application module
        out: applgateOut; // to application module
endsimple

