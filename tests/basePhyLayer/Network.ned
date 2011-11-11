package org.mixim.tests.basePhyLayer;

import org.mixim.base.modules.BaseNetwork;


network tbpSim extends BaseNetwork
{
    parameters:
        int numHosts; // total number of hosts in the network
        int run; // total number of hosts in the network
        bool showPassed;
    submodules:
        testManager: BasePhyTests {
            @display("p=54,97");
        }
        node[numHosts]: TestHost {
            parameters:
                numHosts = numHosts;
                @display("p=149,96;i=laptop");

        }
}

