package org.mixim.tests.basePhyLayer;

import org.mixim.modules.node.WirelessNode;

module TestHost extends WirelessNode
{
    parameters:
        nicType = "org.mixim.tests.basePhyLayer.TestNic";
        applicationType = "";
        
        netwl.headerLength = 12bit;
        
        int numHosts; // total number of hosts in the network

    connections allowunconnected:
}

