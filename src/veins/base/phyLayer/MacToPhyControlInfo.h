#pragma once

#include "veins/veins.h"

#include "veins/base/toolbox/Signal.h"

namespace Veins {

class Signal;

/**
 * @brief Stores information which is needed by the physical layer
 * when sending a MacPkt.
 *
 * @ingroup phyLayer
 * @ingroup macLayer
 */
class VEINS_API MacToPhyControlInfo : public cObject {
protected:
    /** @brief A pointer to the signal representing the transmission.*/
    Signal* signal;

public:
    /**
     * @brief Initialize the MacToPhyControlInfo with the passed
     * signal or null if signal is ommited.
     *
     * NOTE: Once a signal is passed to the MacToPhyControlInfo,
     *          MacToPhyControlInfo takes the ownership of the Signal.
     */
    MacToPhyControlInfo(Signal* signal = nullptr)
        : signal(signal)
    {
    }

    /**
     * @brief Delete the signal if it is still in our ownership.
     */
    ~MacToPhyControlInfo() override
    {
        if (signal != nullptr) delete signal;
    }

    /**
     * @brief extracts the signal from "control info".
     */
    static Signal* const getSignalFromControlInfo(cObject* const pCtrlInfo)
    {
        MacToPhyControlInfo* const cCtrlInfo = dynamic_cast<MacToPhyControlInfo* const>(pCtrlInfo);
        if (cCtrlInfo == nullptr) return nullptr;

        Signal* tmp_signal = cCtrlInfo->signal;
        cCtrlInfo->signal = nullptr;
        return tmp_signal;
    }
};

} // namespace Veins
