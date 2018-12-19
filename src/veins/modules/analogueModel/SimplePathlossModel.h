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
    </AnalogueModel>
   @endverbatim
 *
 * @ingroup analogueModels
 */
class VEINS_API SimplePathlossModel : public AnalogueModel {
protected:
    /** @brief Path loss coefficient. **/
    double pathLossAlphaHalf;

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
     * @param owner pointer to the cComponent that owns this AnalogueModel
     * @param alpha the coefficient alpha (specified e.g. in config.xml and
     *                 passed in constructor call)
     * @param useTorus information about the playground the host is moving in
     * @param playgroundSize information about the playground the host is
     *                          moving in
     */
    SimplePathlossModel(cComponent* owner, double alpha, bool useTorus, const Coord& playgroundSize)
        : AnalogueModel(owner)
        , pathLossAlphaHalf(alpha * 0.5)
        , useTorus(useTorus)
        , playgroundSize(playgroundSize)
    {
    }

    /**
     * @brief Filters a specified AirFrame's Signal by adding an attenuation
     * over time to the Signal.
     */
    void filterSignal(Signal*) override;

    bool neverIncreasesPower() override
    {
        return true;
    }
};

} // namespace Veins
