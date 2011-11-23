package org.mixim.modules.nic;

//
// This NIC interface simulates an IEEE 802.15.4A UWB PHY
// transceiver. The MAC does not attempt to be standard 
// compliant. 
//
// @author Jérôme Rousselot
//
// See the following publications for more information:
// [1] An Ultra-Wideband Impulse Radio PHY Layer Model for Network Simulation,
// J. Rousselot, J.-D. Decotignie. Simulation: Transactions of the Society
// for Computer Simulation, vol. 87 no. 1-2 pp.82-112, January 2011.
// <a href="http://dx.doi.org/10.1177/0037549710377767">http://dx.doi.org/10.1177/0037549710377767</a>
// [2] A High-Precision Ultra Wideband Impulse Radio Physical Layer Model
// for Network Simulation, J. Rousselot, J.-D. Decotignie,
// Second International Omnet++ Workshop,Simu'TOOLS, Rome, 6 Mar 09.
// <a href="http://portal.acm.org/citation.cfm?id=1537714">http://portal.acm.org/citation.cfm?id=1537714</a>
// [3] Ultra Low Power Communication Protocols for UWB Impulse Radio Wireless Sensor Networks,
// J. Rousselot. EPFL thesis 4720, 2010.
// <a href="http://infoscience.epfl.ch/record/147987">http://infoscience.epfl.ch/record/147987</a> 
//
module Nic802154A extends WirelessNicBattery
{
    parameters:
        macType = default("AlohaMacLayer");
        phyType = default("PhyLayerUWBIR");
        
        // power consumption from boards (at 3.3V)
        sleepCurrent = .00303 mA; // 0.021 µA
        rxCurrent  = 9.1 mA;
        txCurrent = 0.303 mA;
        setupRxCurrent = 9.1 mA;
        setupTxCurrent = 0.303 mA;
        rxTxCurrent = 9.1 mA; 
        txRxCurrent = 9.1 mA; 
        decodingCurrentDelta = 0.0 mA; //not used
        double syncCurrent @unit(mA) = default(30.3 mA);

        phy.initialRadioState = 4; // initializes radio state to SYNC
        mac.maxRetries = 3;
}

