package org.mixim.modules.phy;

// Physical layer module which knows how to initialize the Deciders
// and AnalogueModels from the modules directory.
// See the classes documentation for details.
simple PhyLayerBattery extends PhyLayer
{
    parameters:
        @class(PhyLayerBattery);
        
        // the number of different power consuming activities the phy layer has
        // change this parameter if your decider or physical layer uses more
        // than the default 5 activities (sleep, rx, tx, switching, decoding)
        int numActivities = default(5);
}
