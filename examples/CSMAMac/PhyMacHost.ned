package org.mixim.examples.CSMAMac;

import org.mixim.modules.node.WirelessNodeNetwl;

module PhyMacHost extends WirelessNodeNetwl
{
    parameters:
        networkType = "org.mixim.examples.CSMAMac.SimpleNetwLayer";
        nicType = "org.mixim.examples.CSMAMac.PhyMacNic";
        int numHosts; // total number of hosts in the network
    connections allowunconnected:
}

