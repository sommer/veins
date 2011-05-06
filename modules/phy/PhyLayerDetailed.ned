package org.mixim.modules.phy;

import org.mixim.base.phyLayer.BasePhyLayer;

simple PhyLayerDetailed extends PhyLayerBattery
{
    parameters:
        @class(PhyLayerDetailed);
        @display("i=device/antennatower");
		double timeONToTX @unit(s) = default(0 s);
		double timeONToRX @unit(s) = default(0 s);
		double timeONToSleep @unit(s) = default(0 s);
		
		double timeSleepToON @unit(s) = default(1 ms);
		double timeTXToON @unit(s) = default(0 s);
		
		double timeRXToON @unit(s) = default(0 s);
		
		double phyHeaderDuration @unit(s) = default(192 us);
}

