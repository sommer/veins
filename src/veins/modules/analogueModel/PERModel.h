#pragma once

#include "veins/veins.h"

#include "veins/base/phyLayer/AnalogueModel.h"

using Veins::AirFrame;

namespace Veins {

/**
 * @brief This class applies a parameterized packet error rate
 * to incoming packets. This allows the user to easily
 * study the robustness of its system to packet loss.
 *
 * @ingroup analogueModels
 *
 * @author Jérôme Rousselot <jerome.rousselot@csem.ch>
 */
class VEINS_API PERModel : public AnalogueModel {
protected:
    double packetErrorRate;

public:
    /** @brief The PERModel constructor takes as argument the packet error rate to apply (must be between 0 and 1). */
    PERModel(cComponent* owner, double per)
        : AnalogueModel(owner)
        , packetErrorRate(per)
    {
        ASSERT(per <= 1 && per >= 0);
    }

    void filterSignal(Signal*) override;
};

} // namespace Veins
