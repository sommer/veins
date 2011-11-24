/* -*- mode:c++ -*- ********************************************************/
//
// Author: Emin Ilker Cetinbas (niw3_at_yahoo_d0t_com)
// Generalization: Andras Varga
// Copyright (C) 2005 Emin Ilker Cetinbas, Andras Varga
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


#ifndef MASS_MOBILITY_H
#define MASS_MOBILITY_H

#include "MiXiMDefs.h"
#include "BaseMobility.h"

/**
 * @brief Models the mobility of with mass, making random motions.
 * See NED file for more info.
 *
 * NOTE: Does not yet support 3-dimensional movement.
 * @ingroup mobility
 * @author Emin Ilker Cetinbas, Andras Varga
 */
class MIXIM_API MassMobility : public BaseMobility
{
  protected:
    // config (see NED file for explanation)
    cPar *changeInterval;
    cPar *changeAngleBy;

    // current state
    double currentSpeed;   ///< speed of the host
    double currentAngle;   ///< angle of linear motion
    Coord step;            ///< calculated from speed, angle and updateInterval

    Coord targetPos;

  public:
    /** @brief The kind field of messages
     *
     * that are used internally by this class have one of these values
     *
     */
    enum MassMobilityMsgKinds {
	MK_CHANGE_DIR = BaseMobility::LAST_BASE_MOBILITY_KIND,
	LAST_MASS_MOBILITY_KIND
    };

    //Module_Class_Members( MassMobility, BaseMobility, 0 );

    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int);

   protected:
    /** @brief Called upon arrival of a self messages*/
    virtual void handleSelfMsg(cMessage *msg);

    /** @brief Move the host*/
    virtual void makeMove();

    virtual void fixIfHostGetsOutside();
};

#endif
