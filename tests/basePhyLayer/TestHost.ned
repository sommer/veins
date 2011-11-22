package org.mixim.tests.basePhyLayer;

import org.mixim.modules.node.WirelessNodeNetwl;

module TestHost extends WirelessNodeNetwl
{
    parameters:
        nicType = "org.mixim.tests.basePhyLayer.TestNic";
        
        netwl.headerLength = 12bit;
        
        int numHosts; // total number of hosts in the network

    connections allowunconnected:
}

