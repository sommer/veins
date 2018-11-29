#pragma once

#include <cstdlib>

#include "veins/veins.h"

#include "veins/base/phyLayer/AnalogueModel.h"
#include "veins/base/modules/BaseWorldUtility.h"

namespace Veins {

using Veins::AirFrame;

class SimplePathlossModel;

/**
 * @brief Basic implementation of a SimplePathlossModel
 *
 * An example config.xml for this AnalogueModel can be the following:
 * @verbatim
    <AnalogueModel type="SimplePathlossModel">
        <!-- Environment parameter of the pathloss formula
             If ommited default value is 3.5-->
        <parameter name="alpha" type="double" value="3.5"/>

        <!-- Carrier frequency of the signal in Hz
             If ommited the carrier frequency from the
             connection manager is taken if available
             otherwise set to default frequency of 2.412e+9-->
        <parameter name="carrierFrequency" type="double" value="2.412e+9"/>
    </AnalogueModel>
   @endverbatim
 *
 * @ingroup analogueModels
 */
class VEINS_API SimplePathlossModel : public AnalogueModel {
protected:
    /** @brief Path loss coefficient. **/
    double pathLossAlphaHalf;

    /** @brief carrier frequency needed for calculation */
    double carrierFrequency;

    /** @brief Information needed about the playground */
    const bool useTorus;

    /** @brief The size of the playground.*/
    const Coord& playgroundSize;

public:
    /**
     * @brief Initializes the analogue model. playgroundSize
     * need to be valid as long as this instance exists.
     *
     * The constructor needs some specific knowledge in order to create
     * its mapping properly:
     *
     * @param alpha the coefficient alpha (specified e.g. in config.xml and
     *                 passed in constructor call)
     * @param carrierFrequency the carrier frequency
     * @param useTorus information about the playground the host is moving in
     * @param playgroundSize information about the playground the host is
     *                          moving in
     */
    SimplePathlossModel(double alpha, double carrierFrequency, bool useTorus, const Coord& playgroundSize)
        : pathLossAlphaHalf(alpha * 0.5)
        , carrierFrequency(carrierFrequency)
        , useTorus(useTorus)
        , playgroundSize(playgroundSize)
    {
    }

    /**
     * @brief Filters a specified AirFrame's Signal by adding an attenuation
     * over time to the Signal.
     */
    void filterSignal(Signal*, const Coord&, const Coord&) override;

    /**
     * @brief Method to calculate the attenuation value for pathloss.
     *
     * Functionality is similar to pathloss-calculation in BasicSnrEval from
     * Mobility-frame work.
     */
    virtual double calcPathloss(const Coord& receiverPos, const Coord& senderPos);

    bool neverIncreasesPower() override
    {
        return true;
    }
};

} // namespace Veins
