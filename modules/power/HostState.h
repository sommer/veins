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

#ifndef HOSTSTATE_H
#define HOSTSTATE_H

#include "BaseUtility.h"

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
 */
class HostState : public BBItem
{
    BBITEM_METAINFO(BBItem)

public:
    enum States
        {
          ON,
          FAILED,
          OFF 		// not used
        };
    // we could make a nice 'info' field here, to allow us to specify
    // the cause of failure (e.g. battery, stochastic hardware failure)

private:
    States state;

public:

    States get() const { return state; }
    void set(States s) { state = s; }
};

#endif
