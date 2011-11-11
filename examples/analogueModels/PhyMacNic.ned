package org.mixim.examples.analogueModels;

import org.mixim.modules.nic.WirelessNic;

module PhyMacNic extends WirelessNic
{
    parameters:
    	macType = "SimpleMacLayer";
    	phyType = "SamplePhyLayer";
}

