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

#include "LineSegmentsMobilityBase.h"

#include <FWMath.h>


void LineSegmentsMobilityBase::beginNextMove(cMessage *msg)
{
    EV << "beginNextMove, startPos: " << move.startPos.info() << " stepTarget: " << stepTarget.info() << " targetPos: " 
       << targetPos.info() << endl << "simTime: " << simTime() << " targetTime: " << targetTime << endl;;

    // go to exact position where previous statement was supposed to finish
    move.startPos = targetPos;
    stepTarget = targetPos;
    simtime_t now = targetTime;

    // choose new targetTime and targetPos
    setTargetPosition();


    EV << "startPos: " << move.startPos.info() << " targetPos: " << targetPos.info() << endl;

    if (targetTime<now)
        error("LineSegmentsMobilityBase: targetTime<now was set in %s's beginNextMove()", className());

    if( move.speed <= 0 ){
        // end of movement
        stepSize.setX(0);
        stepSize.setY(0);
	EV << "speed < 0; stop moving!\n";
        delete msg;
    }
    else if (targetPos==move.startPos){
        // no movement, just wait
	EV << "warning, we are not moving!\n";
        stepSize.setX(0);
        stepSize.setY(0);
        scheduleAt(Max(targetTime,simTime()), msg);
    }
    else{
        // keep moving
        double numIntervals = (targetTime-now) / updateInterval;
        // int numSteps = floor(numIntervals); -- currently unused,
        // although we could use step counting instead of comparing
        // simTime() to targetTime each step.

        // Note: step = speed*updateInterval = distance/time*updateInterval =
        //        = (targetPos-pos) / (targetTime-now) * updateInterval =
        //        = (targetPos-pos) / numIntervals
        stepSize = (targetPos - move.startPos);

	move.setDirection( targetPos );

	move.startTime = simTime();

	stepSize = stepSize / numIntervals;
	stepTarget += stepSize;

	move.speed = move.startPos.distance( targetPos ) / (targetTime - now );

	EV << "numIntervals: " << numIntervals << " now: " << now << " simTime " << simTime() << " stepTarget: " 
	   << stepTarget.info() << " speed: " << move.speed << endl; 

        scheduleAt(simTime() + updateInterval, msg);
    }
}

void LineSegmentsMobilityBase::handleSelfMsg(cMessage *msg)
{
    if( move.speed <= 0 ){
        delete msg;
        return;
    }
    else if (simTime()+updateInterval >= targetTime){
        beginNextMove(msg);
    }
    else{
	EV << "make next step\n";

        scheduleAt(simTime() + updateInterval, msg);

	// update position
	move.startPos = stepTarget;
	stepTarget += stepSize;
	move.startTime = simTime();
    }

    // do something if we reach the wall
    fixIfHostGetsOutside();

    updatePosition();
}


