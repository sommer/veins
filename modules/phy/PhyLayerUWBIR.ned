package org.mixim.modules.phy;

import org.mixim.base.phyLayer.BasePhyLayer;

//
// A physical layer that models an Ultra Wideband Impulse Radio wireless communication system.
//
// This class loads channel models and deliver frames to an UWB Decider. It is independent of the modulation technique,
// as long as the frames are represented using the same approach as in IEEE802154A.h (Maximum Pulse Amplitude Estimation).
//
// Several channel models are possible: Ghassemzadeh-LOS, Ghassemadeh-NLOS (see UWBIRStochasticPathlossModel.h)
// and the IEEE 802.15.4A UWB channel models that use the default power delay profile (see UWBIRIEEE802154APathlossModel.h).
//
// Currently, an energy detection receiver is modeled in UWBIRED.h.
// Several synchronization logics have been implemented in derived classes:
// see DeciderUWBIREDSync.h and and DeciderUWBIREDSyncOnAddress.h.
//
// To add a novel receiver (e.g. coherent demodulation), either derive UWBIRED or write a new one,
// then add functionality in this module to load the new decider.
// The same procedure applies for new channel models.
//
// To change the modulation, refer to UWBIRMac.h, IEEE802154A.h and UWBIRED.h.
//
// To implement optional modes of IEEE802154A, refer to IEEE802154A.h.
//
// Refer to the following publications for more information:
//
// [1] An Ultra Wideband Impulse Radio PHY Layer Model for Network Simulation,
// J. Rousselot, J.-D. Decotignie, Simulation: Transactions of the Society
// for Computer Simulation, 2010 (to appear).
//
// http://sim.sagepub.com/archive/2010.dtl
//
// [2]  A High-Precision Ultra Wideband Impulse Radio Physical Layer Model
// for Network Simulation, Jérôme Rousselot, Jean-Dominique Decotignie,
// Second International Omnet++ Workshop,Simu'TOOLS, Rome, 6 Mar 09.
//
// http://portal.acm.org/citation.cfm?id=1537714
//
simple PhyLayerUWBIR extends BasePhyLayer
{
    parameters:
        @class(PhyLayerUWBIR);
        @display("i=device/antennatower");
}

