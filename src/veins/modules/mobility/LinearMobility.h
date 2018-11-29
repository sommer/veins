/* -*- mode:c++ -*- ********************************************************
 * file:        LinearMobility.h
 *
 * author:      Emin Ilker Cetinbas (niw3_at_yahoo_d0t_com)
 *
 * Copyright    (C) 2005 Emin Ilker Cetinbas
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/

#pragma once

#include "veins/base/modules/BaseMobility.h"

namespace Veins {

/**
 * @brief Linear movement model. See NED file for more info.
 *
 * This mobility module expects a torus as playground ("useTorus"
 * Parameter of BaseWorldUtility module).
 *
 * NOTE: Does not yet support 3-dimensional movement.
 * @ingroup mobility
 * @author Emin Ilker Cetinbas
 */
class VEINS_API LinearMobility : public BaseMobility {
protected:
    double angle; ///< angle of linear motion
    double acceleration; ///< acceleration of linear motion

    /** @brief always stores the last step for position display update */
    Coord stepTarget;

public:
    /** @brief Initializes mobility model parameters.*/
    void initialize(int) override;

protected:
    /** @brief Move the host*/
    void makeMove() override;

    void fixIfHostGetsOutside() override;
};

} // namespace Veins
