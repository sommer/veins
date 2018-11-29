/* -*- mode:c++ -*- ********************************************************
 * Energy Framework for Omnet++, version 0.9
 *
 * Author:  Laura Marie Feeney
 *
 * Copyright 2009 Swedish Institute of Computer Science.
 *
 * This software is provided `as is' and without any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.
 *
 ***************************************************************************/

#pragma once

#include "veins/veins.h"

namespace Veins {

/**
 * @brief HostState is published by the battery to announce host failure
 *
 * HostState information is published by the Battery to announce Host
 * failure due to battery depletion.  (Default state is ON, OFF is not
 * used; existing modules basically ignore everything but FAIL.)
 *
 * All modules that generate messages and collect statistics should
 * subscribe to this notification.  Should eventually be generalized
 * to handle more general HostState (e.g. stochastic failure,
 * restart).
 *
 * @ingroup power
 */
class VEINS_API HostState : public cObject {
public:
    /**
     * @brief Possible host states.
     */
    enum States {
        ACTIVE, /** Host is active and fully working*/
        FAILED, /** Host is not active because of some failure
                 * but might able to be restarted*/
        BROKEN, /** Host is not active because of some failure
                 * and won't be able to be restarted */
        SLEEP, /** Host is not active (sleeping) */
        OFF /** Host is not active (shut down) */
    };
    // we could make a nice 'info' field here, to allow us to specify
    // the cause of failure (e.g. battery, stochastic hardware failure)

private:
    /**
     * @brief Host state.
     */
    States state;

public:
    /**
     * @brief Constructor taking a state.
     */
    HostState(States state = ACTIVE)
        : state(state)
    {
    }

    /** @brief Returns the host state */
    States get() const
    {
        return state;
    }
    /** @brief Sets the host state */
    void set(States s)
    {
        state = s;
    }

    /**
     * @brief Returns information about the current state.
     */
    std::string info() const override
    {
        std::ostringstream ost;
        switch (state) {
        case ACTIVE:
            ost << "ACTIVE";
            break;
        case FAILED:
            ost << "FAILED";
            break;
        case BROKEN:
            ost << "BROKEN";
            break;
        case SLEEP:
            ost << "SLEEP";
            break;
        case OFF:
            ost << "OFF";
            break;
        default:
            ost << "Unknown";
            break;
        }
        return ost.str();
    }
};

} // namespace Veins
