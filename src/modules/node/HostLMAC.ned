package org.mixim.modules.node;

//
// Example host module using a LMAC Network interface card.
//
module HostLMAC extends WirelessNodeBattery
{
    parameters:
        nicType = default("org.mixim.modules.nic.NicLMAC");
        arpType = default("org.mixim.modules.netw.ArpHost");
        @display("i2=status/battery");
}
