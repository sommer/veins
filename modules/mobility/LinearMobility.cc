//
// Author: Emin Ilker Cetinbas (niw3_at_yahoo_d0t_com)
// Copyright (C) 2005 Emin Ilker Cetinbas
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

#include "LinearMobility.h"

#include <FWMath.h>


Define_Module(LinearMobility);

void LinearMobility::initialize(int stage)
{
    BaseMobility::initialize(stage);

    EV << "initializing LinearMobility stage " << stage << endl;

    if (stage == 0){
	
        move.speed = par("speed");
        acceleration = par("acceleration");
        angle = par("angle");
        angle = fmod(angle,360);
    }
    else if(stage == 1){
	stepTarget = move.startPos;
    }
}


void LinearMobility::fixIfHostGetsOutside()
{
    Coord dummy;
    handleIfOutside(WRAP, stepTarget, dummy, dummy, angle);
}


/**
 * Move the host if the destination is not reached yet. Otherwise
 * calculate a new random position
 */
void LinearMobility::makeMove()
{
    EV << "start makeMove " << move.info() << endl;

    move.startPos = stepTarget;
    move.startTime = simTime();

    stepTarget.setX(move.startPos.getX() + move.speed * cos(PI * angle / 180) * updateInterval);
    stepTarget.setY(move.startPos.getY() + move.speed * sin(PI * angle / 180) * updateInterval);

    move.setDirection(stepTarget);

    EV << "new stepTarget: " << stepTarget.info() << endl;

    // accelerate
    move.speed += acceleration * updateInterval;

    fixIfHostGetsOutside();
}
