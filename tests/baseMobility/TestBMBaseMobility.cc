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

	BaseMobility::initialize(stage);

	if (stage == 0) {
		updateInterval = 0.1;
		move.setSpeed(0.1);
		testStage = 0;
		testsPassed = true;
		use2D = world->use2D();

		ignoreNext = 0;

		pgsX = playgroundSizeX();
		pgsY = playgroundSizeY();
		pgsZ = playgroundSizeZ();
    }
    else if( stage == 1 ){
    	fixIfHostGetsOutside();
    }

}

Coord TestBMBaseMobility::getCoord(double x, double y, double z) {
	if(use2D) {
		return Coord(x, y);
	} else {
		return Coord(x, y, z);
	}
}

void TestBMBaseMobility::calcTestReflection(Coord from, Coord collission, Coord reflected) {
	move.setStart(from, move.getStartTime());
	move.setDirectionByTarget(collission);
	move.setSpeed((from - collission).length() * 2.0);

	expectedBorderTime = simTime() + 0.5;
	expectedBorderMove.setStart(collission, expectedBorderMove.getStartTime());
	expectedBorderMove.setDirectionByTarget(reflected);

	policy = REFLECT;

	step = move.getDirection() * move.getSpeed();
	expStep = expectedBorderMove.getDirection() * move.getSpeed();
	stepTarget = move.getStartPos() + step;
	expStepTarget = expectedBorderMove.getStartPos() + expStep * 0.5;
	targetPos = move.getStartPos() + step * 2.0;
	expTargetPos = expectedBorderMove.getStartPos() + expStep * 1.5;
}

void TestBMBaseMobility::getNextTestMove() {
	move.setStart(move.getStartPos(), simTime());

	switch(++testStage) {
	case 1:
		//reflect at lower x border
		calcTestReflection(getCoord(0.5, pgsY * 0.5, pgsZ * 0.5),
						   getCoord(0.0, pgsY * 0.5, pgsZ * 0.5),
						   getCoord(0.5, pgsY * 0.5, pgsZ * 0.5));


		break;
	case 2:
		//reflect at bigger x border
		calcTestReflection(getCoord(pgsX - 0.5, pgsY * 0.5, pgsZ * 0.5),
						   getCoord(pgsX, pgsY * 0.5, pgsZ * 0.5),
						   getCoord(pgsX - 0.5, pgsY * 0.5, pgsZ * 0.5));


		break;
	case 3:
		//reflect at lower y border
		calcTestReflection(getCoord(pgsX * 0.5, 0.5, pgsZ * 0.5),
						   getCoord(pgsX * 0.5, 0.0, pgsZ * 0.5),
						   getCoord(pgsX * 0.5, 0.5, pgsZ * 0.5));

		break;

	case 4:
		//reflect at bigger y border
		calcTestReflection(getCoord(pgsX * 0.5, pgsY - 0.5, pgsZ * 0.5),
						   getCoord(pgsX * 0.5, pgsY, pgsZ * 0.5),
						   getCoord(pgsX * 0.5, pgsY - 0.5, pgsZ * 0.5));

		break;
	case 5:
		//reflect at lower x and y border
		calcTestReflection(getCoord(0.5, 0.5, pgsZ * 0.5),
						   getCoord(0.0, 0.0, pgsZ * 0.5),
						   getCoord(0.5, 0.5, pgsZ * 0.5));
		ignoreNext = 1;
		break;

	case 6:
		//reflect at lower x and bigger y border
		calcTestReflection(getCoord(0.5, pgsY - 0.5, pgsZ * 0.5),
						   getCoord(0.0, pgsY, pgsZ * 0.5),
						   getCoord(0.5, pgsY - 0.5, pgsZ * 0.5));
		ignoreNext = 1;
		break;

	case 7:
		//reflect at bigger x and bigger y border
		calcTestReflection(getCoord(pgsX - 0.5, pgsY - 0.5, pgsZ * 0.5),
						   getCoord(pgsX, pgsY, pgsZ * 0.5),
						   getCoord(pgsX - 0.5, pgsY - 0.5, pgsZ * 0.5));
		ignoreNext = 1;
		break;

	case 8:
		//reflect at bigger x and lower y border
		calcTestReflection(getCoord(pgsX - 0.5, 0.5, pgsZ * 0.5),
						   getCoord(pgsX, 0.0, pgsZ * 0.5),
						   getCoord(pgsX - 0.5, 0.5, pgsZ * 0.5));
		ignoreNext = 1;
		break;

	case 9:
		//cornerreflection should always reflected to source test
		calcTestReflection(getCoord(2.372, 1.211, pgsZ * 0.5),
						   getCoord(pgsX, 0.0, pgsZ * 0.5),
						   getCoord(2.372, 1.211, pgsZ * 0.5));
		ignoreNext = 1;
		break;

	case 10:
		//3-dimensional test from here set breakpoint for 2D
		if(use2D) {
			move.setSpeed(0.0);
			break;
		}

		//reflect at lower z border
		calcTestReflection(getCoord(pgsX * 0.5, pgsY * 0.5, 0.5),
						   getCoord(pgsX * 0.5, pgsY * 0.5, 0.0),
						   getCoord(pgsX * 0.5, pgsY * 0.5, 0.5));
		break;
	case 11:
		//reflect at upper z and lower x border
		calcTestReflection(getCoord(0.231, pgsY * 0.5, pgsZ - 0.24),
						   getCoord(0.0, pgsY * 0.5, pgsZ),
						   getCoord(0.231, pgsY * 0.5, pgsZ - 0.24));
		ignoreNext = 1;
		break;

	case 12:
		//reflect at upper z and upper x and upper y border
		calcTestReflection(getCoord(pgsX - 1.5, pgsY - 0.5, pgsZ - 0.5),
						   getCoord(pgsX, pgsY, pgsZ),
						   getCoord(pgsX - 1.5, pgsY - 0.5, pgsZ - 0.5));
		ignoreNext = 2;
		break;

/*
 	case 11:
		//reflect at upper z and lower x border
		calcTestReflection(getCoord(pgsX * 0.5, pgsY * 0.5, pgsZ * 0.5),
						   getCoord(pgsX * 0.5, pgsY * 0.5, pgsZ * 0.5),
						   getCoord(pgsX * 0.5, pgsY * 0.5, pgsZ * 0.5));
		ignoreNext = 1;
		break;
*/


	default:
		move.setSpeed(0.0);
		updateInterval = 0.0;
		break;
	}

}

void TestBMBaseMobility::finish()
{
	assertTrue("Checking if all mobility tests passed.", testsPassed);
}


void TestBMBaseMobility::assertRightTime(simtime_t_cref expected) {
	if(fabs(expected - simTime()) < 0.00001) {
		ev << "Passed: Check for correct arrival time of border message " << testStage << endl;
	} else {
		ev << "FAILED: Check for correct arrival time of border message " << testStage <<
			  ". Expeted " << expected << " was " << simTime() << endl;
		testsPassed = false;
	}
}

void TestBMBaseMobility::assertRightMove(Move& expected) {
	if(expected.getStartPos() == move.getStartPos()) {
		ev << "Passed: Check for correct start position of border move " << testStage << endl;
	} else {
		ev << "FAILED: Check for correct start position of border move " << testStage <<
			  ". Expected " << expected.getStartPos().info() << " was " << move.getStartPos().info() << endl;

		testsPassed = false;
	}

	if(expected.getDirection() == move.getDirection()) {
		ev << "Passed: Check for correct direction of border move " << testStage << endl;
	} else {
		ev << "FAILED: Check for correct direction of border move " << testStage <<
			  ". Expected " << expected.getDirection().info() << " was " << move.getDirection().info() << endl;

		testsPassed = false;
	}
}

/**
 * Move the host if the destination is not reached yet. Otherwise
 * calculate a new random position
 */
void TestBMBaseMobility::makeMove()
{
    //we wont do anything :P
}


void TestBMBaseMobility::fixIfHostGetsOutside()
{
	if(ignoreNext == 0){
		if(simTime() != 0.0) {
			assertRightTime(expectedBorderTime);
			assertRightMove(expectedBorderMove);
		}
	    getNextTestMove();
	    if(move.getSpeed() == 0.0) {
	    	return;
	    }
	} else {
		ignoreNext--;
	}
    assertTrue("In our test every handleIfOutside should find a collission.",
    		   handleIfOutside( policy, stepTarget, targetPos, step, angle ));

    if(ignoreNext == 0) {
    	assertRightBorderHandling();
    }
}

void TestBMBaseMobility::assertRightBorderHandling() {
	if(expStepTarget == stepTarget) {
		ev << "Passed: Check for correct step target of border handling " << testStage << endl;
	} else {
		ev << "FAILED: Check for correct step target of border handling " << testStage <<
			  ". Expected " << expStepTarget.info() << " was " << stepTarget.info() << endl;

		testsPassed = false;
	}

	if(expTargetPos == targetPos) {
		ev << "Passed: Check for correct target position of border handling " << testStage << endl;
	} else {
		ev << "FAILED: Check for correct target position of border handling " << testStage <<
			  ". Expected " << expTargetPos.info() << " was " << targetPos.info() << endl;

		testsPassed = false;
	}

	if(expStep == step) {
		ev << "Passed: Check for correct step of border handling " << testStage << endl;
	} else {
		ev << "FAILED: Check for correct step of border handling " << testStage <<
			  ". Expected " << expStep.info() << " was " << step.info() << endl;

		testsPassed = false;
	}

	/*if(expAngle == angle) {
		ev << "Passed: Check for correct angle of border handling " << testStage << endl;
	} else {
		ev << "FAILED: Check for correct agnel of border handling " << testStage <<
			  ". Expected " << expAngle << " was " << angle << endl;

		testPassed = false;
	}*/
}

