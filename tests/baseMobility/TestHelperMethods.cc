#include "TestHelperMethods.h"
#include <FWMath.h>

Define_Module(TestHelperMethods);


void TestHelperMethods::initialize(int stage)
{
	TestBaseMobility::initialize(stage);
	
	if (stage == 2)
	{
		note << "This host is not moving, it just tests helper-methods of its BaseMobility-class" << endl;
		
		// read BorderPolicy parameter from ini-file
		
		int bPolValue = 0;
		if ( hasPar("borderPolicy") )
		{ 
			bPolValue = par("borderPolicy");	
			
		}
		
		// set border policy properly		
		switch (bPolValue) {
				case 1:
					bPol = REFLECT;
					break;
				case 2:
					bPol = WRAP;
					break;
				case 3:
					bPol = PLACERANDOMLY;
					break;
				default:
					bPol = RAISEERROR;
					break;
			}
		
		bPolStr = "";
		setBPolStr(bPolStr, bPol);		
		note << "Using " << bPolStr << " as BorderPolicy." << endl;
		
		// initializing border handling
		bHand = NOWHERE;
		
		bHandStr = "";
		setBHandStr(bHandStr, bHand);
		note << "BorderHandling initially set to " << bHandStr << endl;
		
		// getting info about PlayGroundSize
		Coord pgs = *(world->getPgs());
		
		use2D = world->use2D();
		
		// setting an outer point
		if ( hasPar("outX") && hasPar("outY") && (hasPar("outZ") || use2D) )
		{
			if (!use2D)	{ pointOutside = Coord( par("outX"), par("outY"), par("outZ") ); }
			else { pointOutside = Coord ( par("outX"), par("outY") ); }
		}
		else
		{
			if (!use2D)	{ pointOutside = pgs + Coord (10, 10, 10); }
			else { pointOutside = pgs + Coord (10, 10); }
		}
		
		// setting values of playground size to member variables
		xMax = pgs.getX();
		yMax = pgs.getY();
		if (!use2D) { zMax = pgs.getZ(); }
		else { zMax = Coord::UNDEFINED; }
		
		xMin = 0.0;
		yMin = 0.0;
		if (!use2D) { zMin = 0.0; }
		else { zMin = Coord::UNDEFINED; }
		
		// some output
		note << "My Move-Info: " << move.info() << endl;
		note << "My Target: " << pointOutside.info() << endl;
		
		note << "Starting tests..." << endl;
		
				
		// Here the methods to be tested are called
		
		
		passed(testCheckIfOutside());
		
		
	}

}

// handling incoming messages 
void TestHelperMethods::handleSelfMsg( cMessage* msg )
{
	delete msg;
	note << "Self-Message dropped. In case of static host you should not see me." << endl;
}

void TestHelperMethods::handleBorderMsg( cMessage* msg)
{
	delete msg;
	note << "Border-Message dropped. In case of static host you should not see me." << endl;
}

// finish method
void TestHelperMethods::finish()
{
	// Evaluate obtained results here
	
}

// output for a bool
void TestHelperMethods::passed(bool b)
{
	if (b) { ev << "PASSED" << endl; }
	else { ev << "FAILED" << endl; }	
}

// interprete int values for borderHandling and border Policy
void TestHelperMethods::setBHandStr(std::string& str, int i){
	
	switch (i) {
		case X_SMALLER:
			str = "X_SMALLER";
			break;
		case X_BIGGER:
			str = "X_BIGGER";
			break;
		case Y_SMALLER:
			str = "Y_SMALLER";
			break;
		case Y_BIGGER:
			str = "Y_BIGGER";
			break;
		case Z_SMALLER:
			str = "Z_SMALLER";
			break;
		case Z_BIGGER:
			str = "Z_BIGGER";
			break;
		case NOWHERE:
			str = "NOWHERE";
			break;
		default:
			break;
	}
	
}

void TestHelperMethods::setBPolStr(std::string& str, int i){
	
	switch (i) {
		case REFLECT:
			str = "REFLECT";
			break;
		case WRAP:
			str = "WRAP";
			break;
		case PLACERANDOMLY:
			str = "PLACERANDOMLY";
			break;
		case RAISEERROR:
			str = "RAISEERROR";
			break;
		default:
			break;
	}
	
}

// test checkIfOutside
bool TestHelperMethods::testCheckIfOutside()
{
	bool passed = true;
	
	passed = passed && testSimpleCIO();
	passed = passed && testComplexCIO();
	passed = passed && testBorderCIO();
	
	return passed;
}

bool TestHelperMethods::testSimpleCIO()
{
	Coord origin(0,0,0);
	Coord borderStep = origin;
	Coord stepTarget;
	double dist = 10.0;
	
	bool passed = true;
	
	note << "Testing simple checkIfOutside... " << endl;
	
	double m = move.startPos.getX();
	
	// test X
	stepTarget = Coord(xMax+dist, m, m);
	move.setDirection(stepTarget);
	note << "My Target: " << stepTarget.info() << endl;
	
	passed = passed &&
		(checkIfOutside(stepTarget, borderStep) == X_BIGGER);
	note << passed << "Step: " << borderStep.info() << endl;
	borderStep = origin;
	
	
	stepTarget = Coord(xMin-dist, m, m);
	move.setDirection(stepTarget);
	note << "My Target: " << stepTarget.info() << endl;
	
	passed = passed &&
		(checkIfOutside(stepTarget, borderStep) == X_SMALLER);
	note << passed << "Step: " << borderStep.info() << endl;
	borderStep = origin;
	
	// test Y
	stepTarget = Coord(m, yMax+dist, m);
	move.setDirection(stepTarget);
	note << "My Target: " << stepTarget.info() << endl;
	
	passed = passed &&
		(checkIfOutside(stepTarget, borderStep) == Y_BIGGER);
	note << passed << "Step: " << borderStep.info() << endl;
	borderStep = origin;
	
	
	stepTarget = Coord(m, yMin-dist, m);
	move.setDirection(stepTarget);
	note << "My Target: " << stepTarget.info() << endl;
	
	passed = passed &&
		(checkIfOutside(stepTarget, borderStep) == Y_SMALLER);
	note << passed << "Step: " << borderStep.info() << endl;
	borderStep = origin;
	
	// test Z
	if (!use2D) {
		
		stepTarget = Coord(m, m, zMax+dist);
		move.setDirection(stepTarget);
		note << "My Target: " << stepTarget.info() << endl;
		
		passed = passed &&
			(checkIfOutside(stepTarget, borderStep) == Z_BIGGER);
		note << passed << "Step: " << borderStep.info() << endl;
		borderStep = origin;
		
		
		stepTarget = Coord(m, m, zMin-dist);
		move.setDirection(stepTarget);
		note << "My Target: " << stepTarget.info() << endl;
		
		passed = passed &&
			(checkIfOutside(stepTarget, borderStep) == Z_SMALLER);
		note << passed << "Step: " << borderStep.info() << endl;
		borderStep = origin;
			
	}

	return passed;
}

bool TestHelperMethods::testComplexCIO()
{
	Coord origin(0,0,0);
	Coord borderStep = origin;
	Coord stepTarget;
	double dist = 30.0;
	
	bool passed = true;
	
	note << "Testing complex checkIfOutside... " << endl;
	
	double m = move.startPos.getX();
	
	// test X
	note << endl;
	note << "Should cross X first." << endl; 
	for (int i = -1; i <= (yMax/dist +1); i++){
		
		for (int j = -1; j <= (zMax/dist +1); j++){
	
			stepTarget = Coord(xMax+2*dist, i*dist, j*dist);
			move.setDirection(stepTarget);
			note << "My Target: " << stepTarget.info() << endl;
			
			passed = passed &&
				(checkIfOutside(stepTarget, borderStep) == X_BIGGER);
			note << passed << " Step to border: " << borderStep.info() << endl;
			borderStep = origin;
		}
	}	
	
	for (int i = -1; i <= (yMax/dist +1); i++){
		
		for (int j = -1; j <= (zMax/dist +1); j++){
	
			stepTarget = Coord(xMin-2*dist, i*dist, j*dist);
			move.setDirection(stepTarget);
			note << "My Target: " << stepTarget.info() << endl;
			
			passed = passed &&
				(checkIfOutside(stepTarget, borderStep) == X_SMALLER);
			note << passed << " Step to border: " << borderStep.info() << endl;
			borderStep = origin;
		}
	}	
	
	// test Y
	note << endl;
	note << "Should cross Y first." << endl; 
	for (int i = -1; i <= (xMax/dist +1); i++){
		
		for (int j = -1; j <= (zMax/dist +1); j++){
	
			stepTarget = Coord(i*dist, yMax+2*dist, j*dist);
			move.setDirection(stepTarget);
			note << "My Target: " << stepTarget.info() << endl;
			
			passed = passed &&
				(checkIfOutside(stepTarget, borderStep) == Y_BIGGER);
			note << passed << " Step to border: " << borderStep.info() << endl;
			borderStep = origin;
		}
	}	
	
	for (int i = -1; i <= (xMax/dist +1); i++){
		
		for (int j = -1; j <= (zMax/dist +1); j++){
	
			stepTarget = Coord(i*dist, yMin-2*dist, j*dist);
			move.setDirection(stepTarget);
			note << "My Target: " << stepTarget.info() << endl;
			
			passed = passed &&
				(checkIfOutside(stepTarget, borderStep) == Y_SMALLER);
			note << passed << " Step to border: " << borderStep.info() << endl;
			borderStep = origin;
		}
	}
	
	// test Z
	note << endl;
	note << "Should cross Z first." << endl; 
	if (!use2D){
		
		for (int i = -1; i <= (xMax/dist +1); i++){
		
			for (int j = -1; j <= (yMax/dist +1); j++){
		
				stepTarget = Coord(i*dist, j*dist, zMax+2*dist);
				move.setDirection(stepTarget);
				note << "My Target: " << stepTarget.info() << endl;
				
				passed = passed &&
					(checkIfOutside(stepTarget, borderStep) == Z_BIGGER);
				note << passed << " Step to border: " << borderStep.info() << endl;
				borderStep = origin;
			}
		}
		
		for (int i = -1; i <= (xMax/dist +1); i++){
		
			for (int j = -1; j <= (yMax/dist +1); j++){
		
				stepTarget = Coord(i*dist, j*dist, zMin-2*dist);
				move.setDirection(stepTarget);
				note << "My Target: " << stepTarget.info() << endl;
				
				passed = passed &&
					(checkIfOutside(stepTarget, borderStep) == Z_SMALLER);
				note << passed << " Step to border: " << borderStep.info() << endl;
				borderStep = origin;
			}
		}		
	}
	
	return passed;
}

bool TestHelperMethods::testBorderCIO()
{	
	Coord origin(0,0,0);
	Coord borderStep = origin;
	Coord stepTarget;
	double dist = 30.0;
	
	bool passed = true;
	
	double min = 0.0;
	int bHandVal;
	
	std::string debug = "";
	
	note << "Testing border and edges checkIfOutside... " << endl;
	
	double m = move.startPos.getX();
	
	// testing origin
	stepTarget = Coord(min, min, min);
	move.setDirection(stepTarget);	
	note << "My Target: " << stepTarget.info() << endl;
				
	passed = passed &&
		(checkIfOutside(stepTarget, borderStep) == NOWHERE);
	note << passed << " Step to border: " << borderStep.info() << endl;
	borderStep = origin;
	
	// "one max corners"
	stepTarget = Coord(xMax, min, min);
	move.setDirection(stepTarget);	
	note << "My Target: " << stepTarget.info() << endl;
				
	passed = passed &&
		(checkIfOutside(stepTarget, borderStep) == X_BIGGER);
	note << passed << " Step to border: " << borderStep.info() << endl;
	borderStep = origin;
	
	
	stepTarget = Coord(min, yMax, min);
	move.setDirection(stepTarget);	
	note << "My Target: " << stepTarget.info() << endl;
				
	passed = passed &&
		(checkIfOutside(stepTarget, borderStep) == Y_BIGGER);
	note << passed << " Step to border: " << borderStep.info() << endl;
	borderStep = origin;
	
	
	stepTarget = Coord(min, min, zMax);
	move.setDirection(stepTarget);	
	note << "My Target: " << stepTarget.info() << endl;
				
	passed = passed &&
		(checkIfOutside(stepTarget, borderStep) == Z_BIGGER);
	note << passed << " Step to border: " << borderStep.info() << endl;
	borderStep = origin;
	
	// "two max corners"
	// this one contains output of the result
	stepTarget = Coord(xMax, yMax, min);
	move.setDirection(stepTarget);	
	note << "My Target: " << stepTarget.info() << endl;
				
	passed = passed &&
		((bHandVal = checkIfOutside(stepTarget, borderStep)) == X_BIGGER);
	note << passed << " Step to border: " << borderStep.info() << endl;
	setBHandStr(debug, bHandVal);
	note << "Result: " << debug << endl;
	borderStep = origin;
	
	
	stepTarget = Coord(xMax, min, zMax);
	move.setDirection(stepTarget);	
	note << "My Target: " << stepTarget.info() << endl;
				
	passed = passed &&
		(checkIfOutside(stepTarget, borderStep) == X_BIGGER);
	note << passed << " Step to border: " << borderStep.info() << endl;
	borderStep = origin;
	
	
	stepTarget = Coord(min, yMax, zMax);
	move.setDirection(stepTarget);	
	note << "My Target: " << stepTarget.info() << endl;
				
	passed = passed &&
		(checkIfOutside(stepTarget, borderStep) == Y_BIGGER);
	note << passed << " Step to border: " << borderStep.info() << endl;
	borderStep = origin;
	
	// "all max corner"
	stepTarget = Coord(xMax, yMax, zMax);
	move.setDirection(stepTarget);	
	note << "My Target: " << stepTarget.info() << endl;
				
	passed = passed &&
		(checkIfOutside(stepTarget, borderStep) == X_BIGGER);
	note << passed << " Step to border: " << borderStep.info() << endl;
	borderStep = origin;
	
	
	return passed;
}