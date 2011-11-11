package org.mixim.tests.basePhyLayer;

import org.mixim.modules.nic.WirelessNic;

module TestNic extends WirelessNic
{
    parameters:
        phyType = "org.mixim.tests.basePhyLayer.TestPhyLayer";
        macType = "org.mixim.tests.basePhyLayer.TestMacLayer";
}

