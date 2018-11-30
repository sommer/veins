#pragma once

#include "veins/veins.h"

#include "veins/modules/utility/ConstsPhy.h"

namespace Veins {

/**
 * Stores information which is needed by the physical layer
 * when sending a MacPkt.
 *
 * @ingroup phyLayer
 * @ingroup macLayer
 */
struct VEINS_API MacToPhyControlInfo11p : public cObject {
    int channelNr; ///< Channel number/index used to select frequency.
    enum PHY_MCS mcs; ///< The modulation and coding scheme to employ for the associated frame.
    double txPower_mW; ///< Transmission power in milliwatts.

    MacToPhyControlInfo11p(int channelNr, enum PHY_MCS mcs, double txPower_mW)
        : channelNr(channelNr)
        , mcs(mcs)
        , txPower_mW(txPower_mW)
    {
    }
};

} // namespace Veins
