package org.mixim.modules.node;

//
// Example host module using a BMAC Network interface card.
//
module HostBMAC extends WirelessNodeBattery
{
    parameters:
        arpType = default("org.mixim.modules.netw.ArpHost");
        nicType = default("org.mixim.modules.nic.NicBMAC");
}
