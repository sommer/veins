package org.mixim.examples.ieee802154Narrow;

import org.mixim.modules.node.WirelessNodeBatteryNetwl;

module PhyMacHost extends WirelessNodeBatteryNetwl
{
    parameters:
        networkType = "org.mixim.examples.ieee802154Narrow.NetworkStackTrafficGen";
        nicType = "Nic802154_TI_CC2420";
        int numHosts; // total number of hosts in the network

    connections allowunconnected:
}

