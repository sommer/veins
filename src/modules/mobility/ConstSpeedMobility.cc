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
        move.speed = par("speed").doubleValue();

        if(move.speed <= 0)
	    move.speed = 0;

	numSteps = 0;
	step = -1;
	stepSize = Coord(0,0,0);

        EV << "Initialize: move speed: " << move.speed << " (" << par("speed").doubleValue() << ")"
           << " pos: " << move.info() << endl;
    }
    else if( stage == 1 ){
	stepTarget = move.startPos;
    }	
}


/**
 * Calculate a new random position and the number of steps the host
 * needs to reach this position
 */
void ConstSpeedMobility::setTargetPosition()
{
    EV << "start setTargetPosistion: " << move.info() << endl;

    do{
	targetPos = getRandomPosition();

	double distance = move.startPos.distance(targetPos);
	simtime_t totalTime = distance / move.speed;
	numSteps = FWMath::round(totalTime / updateInterval);

	EV << "new targetPos: " << targetPos.info() << " distance=" << distance 
	   << " totalTime=" << totalTime << " numSteps=" << numSteps << endl;
    }
    while( numSteps == 0 );

    stepSize = targetPos - move.startPos;

    stepSize = stepSize / numSteps;

    stepTarget = move.startPos + stepSize;

    EV << "stepSize: " << stepSize.info() << " target: " << (stepSize*numSteps).info() << endl;

    step = 0;
    move.setDirection(targetPos);

    EV << "end setTargetPosistion: " << move.info() << endl;
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
	move.startPos = stepTarget;
	move.startTime = simTime();

	EV << "stepping forward. step #=" << step
	   << " startPos: " << move.startPos.info() << endl;


	// get new target position
	EV << "destination reached.\n"
	   << move.info() << endl;
	setTargetPosition();
    }
    else if( step < numSteps ){
	// step forward
	move.startPos = stepTarget;
	stepTarget += stepSize;
	move.startTime = simTime();

	EV << "stepping forward. step #=" << step
	   << " startPos: " << move.startPos.info() << endl;

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
