package org.car2x.veins.base.modules;

// Base module for every mac layer module
moduleinterface IWirelessMac
{
    parameters:
        @class(BaseMacLayer);
        int headerLength @unit(bit);              // length of the MAC packet header (in bits)
        string address;                           // MAC address as hex string (12 hex digits), or
                                                  // "auto". "auto" values will be replaced by
                                                  // a generated MAC address in init stage 1.
        
    gates:
        input upperLayerIn; // from upper layer
        output upperLayerOut; // to uppe layer
        input upperControlIn; // control from upper layer
        output upperControlOut; // control to upper layer
        input lowerLayerIn; // from lower layer
        output lowerLayerOut; // to lower layer
        input lowerControlIn; // control from lower layer
        output lowerControlOut; // control to lower layer
}
