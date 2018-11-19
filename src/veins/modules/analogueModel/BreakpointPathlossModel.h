#pragma once

#include <cstdlib>

#include "veins/veins.h"

#include "veins/base/phyLayer/AnalogueModel.h"

using Veins::AirFrame;

namespace Veins {

/**
 * @brief Basic implementation of a BreakpointPathlossModel.
 * This class can be used to implement the ieee802154 path loss model.
 *
 * @ingroup analogueModels
 */
class VEINS_API BreakpointPathlossModel : public AnalogueModel {
protected:
    //    /** @brief Model to use for distances below breakpoint distance */
    //    SimplePathlossModel closeRangeModel;
    //    /** @brief Model to use for distances larger than the breakpoint distance */
    //    SimplePathlossModel farRangeModel;

    /** @brief initial path loss in dB */
    double PL01, PL02;
    /** @brief initial path loss */
    double PL01_real, PL02_real;

    /** @brief pathloss exponents */
    double alpha1, alpha2;

    /** @brief Breakpoint distance squared. */
    double breakpointDistance;

    /** @brief Carrier frequency in Hz */
    double carrierFrequency;

    /** @brief Information needed about the playground */
    const bool useTorus;

    /** @brief The size of the playground.*/
    const Coord& playgroundSize;

    /** logs computed pathlosses. */
    cOutVector pathlosses;

public:
    /**
     * @brief Initializes the analogue model. playgroundSize
     * need to be valid as long as this instance exists.
     */
    BreakpointPathlossModel(double L01, double L02, double alpha1, double alpha2, double breakpointDistance, double carrierFrequency, bool useTorus, const Coord& playgroundSize)
        : PL01(L01)
        , PL02(L02)
        , alpha1(alpha1)
        , alpha2(alpha2)
        , breakpointDistance(breakpointDistance)
        , carrierFrequency(carrierFrequency)
        , useTorus(useTorus)
        , playgroundSize(playgroundSize)
    {
        PL01_real = pow(10, PL01 / 10);
        PL02_real = pow(10, PL02 / 10);
        pathlosses.setName("pathlosses");
    }

    /**
     * @brief Filters a specified AirFrame's Signal by adding an attenuation
     * over time to the Signal.
     */
    virtual void filterSignal(Signal*, const Coord&, const Coord&);

    virtual bool isActiveAtDestination()
    {
        return true;
    }

    virtual bool isActiveAtOrigin()
    {
        return false;
    }
};

} // namespace Veins
