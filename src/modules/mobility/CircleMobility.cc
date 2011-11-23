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

    debugEV << "initializing CircleMobility stage " << stage << endl;

    if (stage == 0) {
        // read parameters
        center.x = (par("cx"));
        center.y = (par("cy"));
        if(!world->use2D()) {
        	center.z = (par("cz"));
        }
        r = par("r");
        ASSERT(r>0);
        angle = par("startAngle").doubleValue()/180.0*PI;
        move.setSpeed(par("speed").doubleValue());
        omega = move.getSpeed()/r;

        // calculate initial position
        if(!world->use2D()) {
        	move.setStart( Coord(center.x + r * cos(angle), center.y + r * sin(angle), center.z) );
        }
        else {
        	move.setStart( Coord(center.x + r * cos(angle), center.y + r * sin(angle)) );
        }

        targetPos = move.getStartPos();
    }
}


void CircleMobility::makeMove()
{
    move.setStart(targetPos, simTime());

    angle += omega * SIMTIME_DBL(updateInterval);
    targetPos.x = (center.x + r * cos(angle));
    targetPos.y = (center.y + r * sin(angle));

    move.setDirectionByTarget(targetPos);

    fixIfHostGetsOutside();
}

void CircleMobility::fixIfHostGetsOutside()
{
    Coord dummy = Coord::ZERO;
    double dum;

    handleIfOutside( WRAP, targetPos, center, dummy, dum);
}
