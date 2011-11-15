package org.mixim.modules.node;

import org.mixim.modules.utility.phyPER;

//
// This host NED module models a host using an IEEE 802.15.4A correlation receiver.
// 
// @author Jérôme Rousselot
// 
// Citation of the following publication is appreciated if you use the MiXiM UWB PHY model
// for a publication of your own.
// J. Rousselot, J.-D. Decotignie, An ultra-wideband impulse radio PHY
// layer model for network simulation. SIMULATION January 2011 vol. 87 no. 1-2 82-112.
//
// For more information, see also:
//
// [1] J. Rousselot, J.-D. Decotignie, An ultra-wideband impulse radio PHY
// layer model for network simulation. SIMULATION January 2011 vol. 87 no.
// 1-2 82-112. <a href="http://dx.doi.org/10.1177/0037549710377767">http://dx.doi.org/10.1177/0037549710377767</a>.
//
// [2] J. Rousselot, Ultra Low Power Communication Protocols for UWB
// Impulse Radio Wireless Sensor Networks. EPFL Thesis 4720, 2010. 
// <a href="http://infoscience.epfl.ch/record/147987">http://infoscience.epfl.ch/record/147987</a>.
//
// [3] J. Rousselot, J.-D. Decotignie, A High-Precision Ultra Wideband Impulse Radio Physical Layer Model
// for Network Simulation. Second International Omnet++ Workshop,Simu'TOOLS, Rome, March 2009.
// <a href="http://portal.acm.org/citation.cfm?id=1537714">http://portal.acm.org/citation.cfm?id=1537714</a>.
//
module Host802154A extends WirelessNodeBattery
{
    parameters:
        applicationType = default("org.mixim.modules.application.TestApplication");
        arpType = default("org.mixim.modules.netw.ArpHost");
        nicType = default("org.mixim.modules.nic.Nic802154A");
        networkType = default("org.mixim.modules.netw.DummyRoute");
        int numHosts; // total number of hosts in the network

    submodules:
        stats: phyPER {
            @display("p=140,310");
        }
}

