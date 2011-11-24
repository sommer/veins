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


#include "MassMobility.h"

#include <FWMath.h>


Define_Module(MassMobility);


/**
 * Reads the updateInterval and the velocity
 *
 * If the host is not stationary it calculates a random position and
 * schedules a timer to trigger the first movement
 */
void MassMobility::initialize(int stage)
{
    BaseMobility::initialize(stage);

    debugEV << "initializing MassMobility stage " << stage << endl;

    if (stage == 0)
    {
        changeInterval = &par("changeInterval");
        changeAngleBy = &par("changeAngleBy");

        // initial speed and angle
        move.setSpeed(par("speed").doubleValue());
        currentAngle = uniform(0, 360);

        step.x = (move.getSpeed() * cos(PI * currentAngle / 180) * SIMTIME_DBL(updateInterval));
        step.y = (move.getSpeed() * sin(PI * currentAngle / 180) * SIMTIME_DBL(updateInterval));

    }
    else if( stage == 1 )
    {
    	if(!world->use2D()) {
			opp_warning("This mobility module does not yet support 3 dimensional movement."\
						"Movements will probably be incorrect.");
		}

		// start moving: set direction and start time
		move.setDirectionByTarget(move.getStartPos() + step);
		move.setStart(move.getStartPos(), simTime());

		targetPos = move.getStartPos();

		scheduleAt(simTime() + uniform(0, changeInterval->doubleValue()), new cMessage("turn", MK_CHANGE_DIR));
    }
}


/**
 * The only self message possible is to indicate a new movement.
 */
void MassMobility::handleSelfMsg(cMessage * msg)
{
    Coord dummy = Coord::ZERO;

    switch (msg->getKind()){
		case MOVE_HOST:
		BaseMobility::handleSelfMsg( msg );
		break;
		case MK_CHANGE_DIR:
		currentAngle += changeAngleBy->doubleValue();

		step.x = (move.getSpeed() * cos(PI * currentAngle / 180) * SIMTIME_DBL(updateInterval));
		step.y = (move.getSpeed() * sin(PI * currentAngle / 180) * SIMTIME_DBL(updateInterval));

		move.setDirectionByTarget(move.getStartPos() + step);

		scheduleAt(simTime() + changeInterval->doubleValue(), msg);
		break;
		default:
		opp_error("Unknown self message kind in MassMobility class");
		break;
    }

}

/**
 * Move the host if the destination is not reached yet. Otherwise
 * calculate a new random position
 */
void MassMobility::makeMove()
{
    move.setStart(targetPos, simTime());
    targetPos += step;

    // do something if we reach the wall
    fixIfHostGetsOutside();
}

void MassMobility::fixIfHostGetsOutside()
{
    Coord dummy = Coord::ZERO;

    handleIfOutside( REFLECT, targetPos, dummy, step, currentAngle );
}
