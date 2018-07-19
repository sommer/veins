#ifndef PATHLOSSMODEL_H_
#define PATHLOSSMODEL_H_

#include <cstdlib>

#include "veins/base/utils/MiXiMDefs.h"
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
class MIXIM_API SimplePathlossModel : public AnalogueModel {
protected:
    /** @brief Path loss coefficient. **/
    double pathLossAlphaHalf;

    /** @brief carrier frequency needed for calculation */
    double carrierFrequency;

    /** @brief Information needed about the playground */
    const bool useTorus;

    /** @brief The size of the playground.*/
    const Coord& playgroundSize;

    /** @brief Whether debug messages should be displayed. */
    bool debug;

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
     * @param debug display debug messages?
     */
    SimplePathlossModel(double alpha, double carrierFrequency, bool useTorus, const Coord& playgroundSize, bool debug)
        : pathLossAlphaHalf(alpha * 0.5)
        , carrierFrequency(carrierFrequency)
        , useTorus(useTorus)
        , playgroundSize(playgroundSize)
        , debug(debug)
    {
    }

    /**
     * @brief Filters a specified AirFrame's Signal by adding an attenuation
     * over time to the Signal.
     */
    virtual void filterSignal(Signal*, const Coord&, const Coord&);

    /**
     * @brief Method to calculate the attenuation value for pathloss.
     *
     * Functionality is similar to pathloss-calculation in BasicSnrEval from
     * Mobility-frame work.
     */
    virtual double calcPathloss(const Coord& receiverPos, const Coord& sendersPos);
};

} // namespace Veins

#endif /*PATHLOSSMODEL_H_*/
