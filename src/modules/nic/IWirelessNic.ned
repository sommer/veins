package org.mixim.modules.nic;

moduleinterface IWirelessNic
{
    @display("i=block/ifcard");
    gates:
        input radioIn; // radioIn gate for sendDirect
        input upperLayerIn; // uppergateIn for incoming messages from network layer
        input upperControlIn; // uppergateCtrlIn for incoming control messages from network layer
        output upperLayerOut; // uppergateOut to send data messages to network layer
        output upperControlOut; // uppergateCtrlOut to send control messages to network layer

}
