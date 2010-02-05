package org.mixim.modules.nic;

moduleinterface INic {
    gates:
        input radioIn; // radioIn gate for sendDirect
        input upperGateIn; // uppergateIn for incoming messages from network layer
        input upperGateCtrlIn; // uppergateCtrlIn for incoming control messages from network layer
        output upperGateOut; // uppergateOut to send data messages to network layer
        output upperGateCtrlOut; // uppergateCtrlOut to send control messages to network layer
        
}