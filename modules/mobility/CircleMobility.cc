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
        move.setSpeed(par("speed"));
        omega = move.getSpeed()/r;

        // calculate initial position
        move.setStart( Coord(center.getX() + r * cos(angle), center.getY() + r * sin(angle)) );

        targetPos = move.getStartPos();
    }
    else
	{
		if(!world->use2D()) {
			opp_warning("This mobility module does not yet support 3 dimensional movement."\
						"Movements will probably be incorrect.");
		}
	}
}


void CircleMobility::makeMove()
{
    move.setStart(targetPos, simTime());

    angle += omega * updateInterval.dbl();
    targetPos.setX(center.getX() + r * cos(angle));
    targetPos.setY(center.getY() + r * sin(angle));

    move.setDirectionByTarget(targetPos);

    fixIfHostGetsOutside();
}

void CircleMobility::fixIfHostGetsOutside()
{
    Coord dummy(world->use2D());
    double dum;

    handleIfOutside( WRAP, targetPos, center, dummy, dum);
}
