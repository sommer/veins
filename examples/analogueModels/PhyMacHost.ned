package org.mixim.examples.analogueModels;

import org.mixim.modules.node.WirelessNode;

module PhyMacHost extends WirelessNode
{
    parameters:
        @display("bgb=,,white,,;bgp=10,10");
        nicType = "PhyMacNic";
        int numHosts; // total number of hosts in the network
}

