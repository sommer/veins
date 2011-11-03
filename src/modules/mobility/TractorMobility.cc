//
// Copyright (C) 2007 Peterpaul Klein Haneveld
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

#include "TractorMobility.h"

#include <FWMath.h>


Define_Module(TractorMobility);


/**
 * Reads the parameters and initializes members.
 */
void TractorMobility::initialize(int stage)
{
	BaseMobility::initialize(stage);

	debugEV << "initializing TractorMobility stage " << stage << endl;

	if (stage == 0)
	{
		x1 = par("x1");
		y1 = par("y1");
		x2 = par("x2");
		y2 = par("y2");
		rows = par("rows");

		move.setSpeed(par("speed").doubleValue());

		dx = x2-x1;
		dy = y2-y1;
		row_length = dx;
		row_width = dy / rows;

		path_length = rows * (row_length + row_width);
		position = 0.0;

		calculateXY();

		move.setStart(targetPos);
	}
	else
	{
		if(!world->use2D()) {
			opp_warning("This mobility module does not yet support 3 dimensional movement."\
						"Movements will probably be incorrect.");
		}
	}
}


void TractorMobility::makeMove()
{
	position += move.getSpeed() * SIMTIME_DBL(updateInterval);

	calculateXY();

	fixIfHostGetsOutside();
}

void TractorMobility::fixIfHostGetsOutside()
{
	Coord dummy = Coord::ZERO;
	double dum;

	handleIfOutside( RAISEERROR, targetPos, dummy, dummy, dum );
}

void TractorMobility::calculateXY()
{
	// update the position
	move.setStart(targetPos, simTime());

	// calculate new target position
	bool moving_away = fmod(position / path_length, 2.0) < 1.0;
	double path_position = fmod(position, (path_length));
	double row_number = fmod(position / (row_length + row_width), rows * 2);
	/* row position can be either in a row or on the edge, so
	 * the range for row_position is [0, row_length + row_width)
	 */
	double row_position = fmod(path_position, (row_length + row_width));
	bool on_the_edge = row_position >= row_length;

	if (moving_away) {
		if (on_the_edge) {
			targetPos.x = (x1 + (fmod(row_number, 2.0) < 1.0 ? row_length : (0)));
            targetPos.y = (y1 + (FWMath::floorToZero(row_number) * row_width + row_position - row_length));
		} else {
			targetPos.x = (x1 + (fmod(row_number, 2.0) < 1.0 ? row_position : (row_length - row_position)));
			targetPos.y = (y1 + (FWMath::floorToZero(row_number) * row_width));
		}
	} else {
		if (on_the_edge) {
			targetPos.x = (x1 + (fmod(row_number, 2.0) < 1.0 ? row_length : (0)));
			targetPos.y = (y2 - ((FWMath::floorToZero(row_number) - rows) * row_width + row_position - row_length));
		} else {
			targetPos.x = (x1 + (fmod(row_number, 2.0) < 1.0 ? row_position : (row_length - row_position)));
			targetPos.y = (y2 - ((FWMath::floorToZero(row_number) - rows) * row_width));
		}
	}

	move.setDirectionByTarget(targetPos);
}
