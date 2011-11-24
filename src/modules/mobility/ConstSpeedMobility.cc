/***************************************************************************
 * file:        ConstSpeedMobility.cc
 *
 * author:      Steffen Sroka
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/


#include "ConstSpeedMobility.h"

#include <FWMath.h>


Define_Module(ConstSpeedMobility);


/**
 * Reads the updateInterval and the velocity
 *
 * If the host is not stationary it calculates a random position and
 * schedules a timer to trigger the first movement
 */
void ConstSpeedMobility::initialize(int stage)
{
    BaseMobility::initialize(stage);

    if (stage == 0) {
        move.setSpeed(par("speed").doubleValue());

        if(move.getSpeed() <= 0)
        	move.setSpeed(0);

		numSteps = 0;
		step = -1;
		stepSize = Coord(0,0,0);

		debugEV << "Initialize: move speed: " << move.getSpeed() << " (" << par("speed").doubleValue() << ")"
           << " pos: " << move.info() << endl;
    }
    else if( stage == 1 ){
    	stepTarget = move.getStartPos();
    }
}


/**
 * Calculate a new random position and the number of steps the host
 * needs to reach this position
 */
void ConstSpeedMobility::setTargetPosition()
{
	debugEV << "start setTargetPosistion: " << move.info() << endl;

    do{
	targetPos = getRandomPosition();

	double distance = move.getStartPos().distance(targetPos);
	simtime_t totalTime = distance / move.getSpeed();
	numSteps = FWMath::round(totalTime / updateInterval);

	debugEV << "new targetPos: " << targetPos.info() << " distance=" << distance
	   << " totalTime=" << totalTime << " numSteps=" << numSteps << endl;
    }
    while( numSteps == 0 );

    stepSize = targetPos - move.getStartPos();

    stepSize = stepSize / numSteps;

    stepTarget = move.getStartPos() + stepSize;

    debugEV << "stepSize: " << stepSize.info() << " target: " << (stepSize*numSteps).info() << endl;

    step = 0;
    move.setDirectionByTarget(targetPos);

    debugEV << "end setTargetPosistion: " << move.info() << endl;
}


/**
 * Move the host if the destination is not reached yet. Otherwise
 * calculate a new random position
 */
void ConstSpeedMobility::makeMove()
{
    // increment number of steps
    step++;

    if( step == numSteps ){
		// last step
		//stepSize.x =
		// step forward
		move.setStart(stepTarget, simTime());

		debugEV << "stepping forward. step #=" << step
		   << " startPos: " << move.getStartPos().info() << endl;


		// get new target position
		debugEV << "destination reached.\n"
		   << move.info() << endl;
		setTargetPosition();
    }
    else if( step < numSteps ){
		// step forward
		move.setStart(stepTarget, simTime());
		stepTarget += stepSize;

		debugEV << "stepping forward. step #=" << step
		   << " startPos: " << move.getStartPos().info() << endl;

    }
    else{
	error("step cannot be bigger than numSteps");
    }

    //    fixIfHostGetsOutside();
}

/*
void ConstSpeedMobility::fixIfHostGetsOutside()
{
    double dummy;

    handleIfOutside( PLACERANDOMLY, stepTarget, targetPos, stepSize, dummy );
}
*/
