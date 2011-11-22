package org.mixim.modules.nic;

moduleinterface IWirelessNicUpperCtrl
{
    @display("i=block/ifcard");
    gates:
        input  radioIn @labels(AirFrame); // radioIn gate for sendDirect
        input  upperLayerIn;              // gate for incoming messages from network layer
        output upperLayerOut;             // gate to send data messages to network layer

        //# following gates are not defined in INET
        
        input  upperControlIn;            // control gate for incoming control messages from network layer
        output upperControlOut;           // control gate to send control messages to network layer
}
