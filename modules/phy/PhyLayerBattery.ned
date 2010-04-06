package org.mixim.modules.phy;

//
// Extends PhyLayer module with power consumption support
//
simple PhyLayerBattery extends PhyLayer
{
    parameters:
        @class(PhyLayerBattery);
        
        // the number of different power consuming activities the phy layer has
        // change this parameter if your decider or physical layer uses more
        // than the default 5 activities (sleep, rx, tx, switching, decoding)
        int numActivities = default(5);
}
