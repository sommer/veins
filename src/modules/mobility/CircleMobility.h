/* -*- mode:c++ -*- ********************************************************/
//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef CIRCLE_MOBILITY_H
#define CIRCLE_MOBILITY_H

#include "MiXiMDefs.h"
#include <BaseMobility.h>

/**
 * @brief Circle movement model. See NED file for more info.
 *
 * @ingroup mobility
 * @author Andras Varga
 */
class MIXIM_API CircleMobility : public BaseMobility
{
  protected:
    // configuration
    Coord center;
    double r;
    double omega;          ///< angular velocity [rad/s], derived from speed and radius

    // state
    double angle;  ///< direction from the centre of the circle

    /** @brief Target position for the host */
    Coord targetPos;

  public:
    //Module_Class_Members( CircleMobility, BaseMobility, 0 );

    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int);

  protected:
    /** @brief Move the host*/
    virtual void makeMove();

    virtual void fixIfHostGetsOutside();
};

#endif

