package org.mixim.modules.phy;

import org.mixim.base.phyLayer.BasePhyLayer;

//
// Physical layer that models an Ultra Wideband Impulse Radio transceiver.
// See the following publication for more information. 
//  A High-Precision Ultra Wideband Impulse Radio Physical Layer Model
// for Network Simulation, Jérôme Rousselot, Jean-Dominique Decotignie, 
// Second International Omnet++ Workshop,Simu'TOOLS, Rome, 6 Mar 09.
// http://portal.acm.org/citation.cfm?id=1537714
//
simple PhyLayerUWBIR extends BasePhyLayer
{
    parameters:
        @class(PhyLayerUWBIR);
        // radio power consumption
        double PSleep;
        double PRx;
        double PTx;
        double PSync;
        double PSwitch;

        @display("i=device/antennatower");

}

