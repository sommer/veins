// The application module for the BaseLocApplLayer.
//
// This module has to be extended in order to implement an application
// which uses localization.
//
// @author Peterpaul Klein Haneveld
simple BaseLocAppl
    parameters:
        debug: bool, // debug switch
        headerLength: numeric const; // length of the application message header (in bits)
    gates:
        in: lowergateIn; // from network layer
        out: lowergateOut; // to network layer
        in: lowerControlIn; // control from network layer
        out: lowerControlOut; // control to network layer
        in: locgateIn; // from localization module
        out: locgateOut; // to localization module
endsimple

