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

#ifndef LINESEGMENTS_MOBILITY_BASE_H
#define LINESEGMENTS_MOBILITY_BASE_H

#include "MiXiMDefs.h"
#include "BaseMobility.h"

/**
 * @brief Base class for mobility models where movement consists of
 * a sequence of linear movements of constant speed.
 *
 * Subclasses must redefine setTargetPosition() which is supposed to set
 * a new target position and target time once the previous one is reached.
 *
 * @ingroup mobility
 * @author Andras Varga
 */
class MIXIM_API LineSegmentsMobilityBase : public BaseMobility
{
  protected:
    /** @name parameters to handle the movement of the host*/
    /*@{*/
    /** @brief Size of a step*/
    Coord stepSize;
    /*@}*/

    // state
    simtime_t targetTime;  ///< end time of current linear movement
    Coord targetPos;    ///< end position of current linear movement
    Coord stepTarget;

  protected:

    /** @brief Called upon arrival of a self messages*/
    virtual void handleSelfMsg(cMessage *msg);

    /** @brief Begin new line segment after previous one finished */
    virtual void beginNextMove(cMessage *msg);

    /**
     * @brief Should be redefined in subclasses. This method gets called
     * when targetPos and targetTime has been reached, and its task is
     * to set a new targetPos and targetTime. At the end of the movement
     * sequence, it should set targetTime=0.
     */
    virtual void setTargetPosition() = 0;

  public:
	  virtual void initialize(int stage);
};

#endif

