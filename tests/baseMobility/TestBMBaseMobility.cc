#include "TestBMBaseMobility.h"

#include <FWMath.h>


Define_Module(TestBMBaseMobility);


/**
 * Reads the updateInterval and the velocity
 *
 * If the host is not stationary it calculates a random position and
 * schedules a timer to trigger the first movement
 */
void TestBMBaseMobility::initialize(int stage){

	TestBaseMobility::initialize(stage);
	
	if (stage == 0) {
        move.speed = par("speed");

        if(move.speed <= 0)
	    move.speed = 0;

	numSteps = 0;
	step = -1;
	stepSize = Coord(0,0,0);

        EV << "Initialize: move speed: " << move.speed << " (" << par("speed") << ")"
           << " pos: " << move.info() << endl;
    }
    else if( stage == 1 ){
	stepTarget = move.startPos;
	
	testsPassed = true;
    }	
	
}

void TestBMBaseMobility::finish()
{
	if (testsPassed) { ev << "All Tests Passed." << endl; }
	else { ev << "FAILED" << endl; }
}


/**
 * Calculate a new random position and the number of steps the host
 * needs to reach this position
 */
void TestBMBaseMobility::setTargetPosition()
{
    EV << "start setTargetPosistion: " << move.info() << endl;

    do{
	targetPos = getOutsidePosition();

	double distance = move.startPos.distance(targetPos);
	double totalTime = distance / move.speed;
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

Coord TestBMBaseMobility::getOutsidePosition()
{
	
	
	if (world->use2D()) {
        
               
        return Coord(genk_uniform(0, 0, playgroundSizeX()) + playgroundSizeX(),
                     genk_uniform(0, 0, playgroundSizeY()) + playgroundSizeY());
    } else {
        return Coord(genk_uniform(0, 0, playgroundSizeX()) + playgroundSizeX(),
                     genk_uniform(0, 0, playgroundSizeY()) + playgroundSizeY(),
                     genk_uniform(0, 0, playgroundSizeZ()) + playgroundSizeZ());
    }	
	
}



/**
 * Move the host if the destination is not reached yet. Otherwise
 * calculate a new random position
 */
void TestBMBaseMobility::makeMove()
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

    fixIfHostGetsOutside();
}


void TestBMBaseMobility::fixIfHostGetsOutside()
{
    double dummy;
    
    handleIfOutside( REFLECT, stepTarget, targetPos, stepSize, dummy );
}

