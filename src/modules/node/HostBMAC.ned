package org.mixim.modules.node;

//
// Example host module using a BMAC Network interface card.
//
module HostBMAC extends WirelessNodeBattery
{
    parameters:
        nicType = default("org.mixim.modules.nic.NicBMAC");
}
