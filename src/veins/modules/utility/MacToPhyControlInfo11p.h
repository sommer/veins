#pragma once

#include "veins/veins.h"

#include "veins/modules/utility/ConstsPhy.h"
#include "veins/modules/utility/Consts80211p.h"

namespace Veins {

/**
 * Stores information which is needed by the physical layer
 * when sending a MacPkt.
 *
 * @ingroup phyLayer
 * @ingroup macLayer
 */
struct VEINS_API MacToPhyControlInfo11p : public cObject {
    Channel channelNr; ///< Channel number/index used to select frequency.
    MCS mcs; ///< The modulation and coding scheme to employ for the associated frame.
    double txPower_mW; ///< Transmission power in milliwatts.

    MacToPhyControlInfo11p(Channel channelNr, MCS mcs, double txPower_mW)
        : channelNr(channelNr)
        , mcs(mcs)
        , txPower_mW(txPower_mW)
    {
    }
};

} // namespace Veins
