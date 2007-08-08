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

#include "CircleMobility.h"

#include <FWMath.h>


Define_Module(CircleMobility);


void CircleMobility::initialize(int stage)
{
    BaseMobility::initialize(stage);

    EV << "initializing CircleMobility stage " << stage << endl;




    if (stage == 0)
    {
        // read parameters
        center.setX(par("cx"));
        center.setY(par("cy"));
        r = par("r");
        ASSERT(r>0);
        angle = par("startAngle").doubleValue()/180.0*PI;
        move.speed = par("speed");
        omega = move.speed/r;

        // calculate initial position
        move.startPos.setX(center.getX() + r * cos(angle));
        move.startPos.setY(center.getY() + r * sin(angle));
	targetPos = move.startPos;
    }
}


void CircleMobility::makeMove()
{
    move.startPos = targetPos;

    angle += omega * updateInterval;
    targetPos.setX(center.getX() + r * cos(angle));
    targetPos.setY(center.getY() + r * sin(angle));

    move.setDirection(targetPos);
    move.startTime = simTime();

    fixIfHostGetsOutside();
}

void CircleMobility::fixIfHostGetsOutside()
{
    Coord dummy;
    double dum;

    handleIfOutside( WRAP, targetPos, center, dummy, dum);
}
