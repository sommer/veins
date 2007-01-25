// The filtering module for BaseLocApplLayer.
//
// This module works as a multiplexer between the network layer on one
// side and BaseLocAppl and BaseLocalization on the other side. Its main
// purpose is to forward messages from the network layer to the right
// module and vice versa.
//
// @author Peterpaul Klein Haneveld
simple LocFilter
    parameters:
        debug: bool, // debug switch
        headerLength: numeric const; // length of the application message header (in bits)
    gates:
        in: lowergateIn; // from network layer
        out: lowergateOut; // to network layer
        in: lowerControlIn; // control from network layer
        out: lowerControlOut; // control to network layer
        in: applgateIn; // from application module
        out: applgateOut; // to application module
        in: applControlIn; // from application module
        out: applControlOut; // to application module
        in: locgateIn; // from localization module
        out: locgateOut; // to localization module
        in: locControlIn; // from localization module
        out: locControlOut; // to localization module
endsimple

