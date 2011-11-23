package org.mixim.examples.multiConnectionManager;

import org.mixim.modules.node.WirelessNode;

module PhyMacHost extends WirelessNode
{
    parameters:
        nicType = "WirelessNic";
        int numHosts; // total number of hosts in the network
}
