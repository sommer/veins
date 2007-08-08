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

#include "RectangleMobility.h"

#include <FWMath.h>


Define_Module(RectangleMobility);


/**
 * Reads the parameters.
 * If the host is not stationary it calculates a random position and
 * schedules a timer to trigger the first movement
 */
void RectangleMobility::initialize(int stage)
{
    BaseMobility::initialize(stage);

    EV << "initializing RectangleMobility stage " << stage << endl;

    if (stage == 0)
    {
        x1 = par("x1");
        y1 = par("y1");
        x2 = par("x2");
        y2 = par("y2");

        move.speed = par("speed");

        // calculate helper vars
        double dx = x2-x1;
        double dy = y2-y1;
        corner1 = dx;
        corner2 = corner1 + dy;
        corner3 = corner2 + dx;
        corner4 = corner3 + dy;

        // determine start position
        double startPos = par("startPos");
        startPos = fmod(startPos,4);
        if (startPos<1)
            d = startPos*dx; // top side
        else if (startPos<2)
            d = corner1 + (startPos-1)*dy; // right side
        else if (startPos<3)
            d = corner2 + (startPos-2)*dx; // bottom side
        else
            d = corner3 + (startPos-3)*dy; // left side
        calculateXY();

	move.startPos = targetPos;

        WATCH(d);
    }
}


void RectangleMobility::makeMove()
{
    d += move.speed * updateInterval;
    while (d<0) d+=corner4;
    while (d>=corner4) d-=corner4;

    calculateXY();

    fixIfHostGetsOutside();
}

void RectangleMobility::fixIfHostGetsOutside()
{
    Coord dummy;
    double dum;

    handleIfOutside( RAISEERROR, targetPos, dummy, dummy, dum );
}

void RectangleMobility::calculateXY()
{
    // update the position
    move.startPos = targetPos;

    // calcultae new target position
    if (d < corner1)
    {
        // top side
        targetPos.setX(x1 + d);
        targetPos.setY(y1);
    }
    else if (d < corner2)
    {
        // right side
        targetPos.setX(x2);
        targetPos.setY(y1 + d - corner1);
    }
    else if (d < corner3)
    {
        // bottom side
        targetPos.setX(x2 - d + corner2);
        targetPos.setY(y2);
    }
    else
    {
        // left side
        targetPos.setX(x1);
        targetPos.setY(y2 - d + corner3);
    }

    move.setDirection(targetPos);
    move.startTime = simTime();
}
