package org.mixim.modules.phy;

import org.mixim.base.phyLayer.BasePhyLayer;

simple PhyLayerDetailed extends PhyLayerBattery
{
    parameters:
        @class(PhyLayerDetailed);
        @display("i=device/antennatower");
		double timeONToTX @unit(s) = default(0 s);
		double timeONToRX @unit(s) = default(0 s);
		double timeSleepToON @unit(s) = default(1 ms);
		int nbRadioChannels = default(1);  // Number of available radio channels. Defaults to single channel radio.
		int initialRadioChannel = default(0);  // Initial radio channel.
}

