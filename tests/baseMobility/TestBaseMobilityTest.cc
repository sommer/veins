#include "TestBaseMobilityTest.h"
#include <FWMath.h>

Define_Module(TestBaseMobilityTest);


void TestBaseMobilityTest::initialize(int stage)
{
	TestBaseMobility::initialize(stage);

	
	if (stage == 2)
	{
		allTestsPassed = true;
		
		//note << "This host is not moving, it just tests helper-methods of its BaseMobility-class" << endl;
		
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
		//note << "Using " << bPolStr << " as BorderPolicy." << endl;
		
		// initializing border handling
		bHand = NOWHERE;
		
		bHandStr = "";
		setBHandStr(bHandStr, bHand);
		//note << "BorderHandling initially set to " << bHandStr << endl;
		
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
		//note << "My Move-Info: " << move.info() << endl;
		//note << "My Target: " << pointOutside.info() << endl;
		
		ev << "Starting tests..." << endl;
		
				
		// Here the methods to be tested are called
		
		testInitialisation();
		testCheckIfOutside();
		
		
	}

}

void TestBaseMobilityTest::testInitialisation() {
	assertTrue("World pointer initialised.", world != 0);
	assertTrue("Base utility pointer initialised.", baseUtility != 0);
	
	assertTrue("Host pointer initialised.", hostPtr != 0);
	assertTrue("Host ID and host pointer matches.", hostPtr->id() == hostId);	
}

// handling incoming messages 
void TestBaseMobilityTest::handleSelfMsg( cMessage* msg )
{
	delete msg;
	//note << "Self-Message dropped. In case of static host you should not see me." << endl;
	assertTrue("Should never receive self messages!", false);
}

void TestBaseMobilityTest::handleBorderMsg( cMessage* msg)
{
	delete msg;
	//note << "Border-Message dropped. In case of static host you should not see me." << endl;
	assertTrue("Should never receive border messages!", false);
}

// finish method
void TestBaseMobilityTest::finish()
{
	assertTrue("Check if all tests passed.", allTestsPassed);
	
}

// output for a bool
void TestBaseMobilityTest::passed(bool b)
{
	if (b) { ev << "PASSED" << endl; }
	else { ev << "FAILED" << endl; }	
}

// interprete int values for borderHandling and border Policy
void TestBaseMobilityTest::setBHandStr(std::string& str, int i){
	
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

void TestBaseMobilityTest::setBPolStr(std::string& str, int i){
	
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
void TestBaseMobilityTest::testCheckIfOutside()
{
	testSimpleCIO();
	testComplexCIO();
	testBorderCIO();
}

void TestBaseMobilityTest::testSimpleCIO()
{
	Coord origin = getCoord(0,0,0);
	Coord borderStep = origin;
	Coord stepTarget;
	double dist = 10.0;
	
	
	ev << "Testing simple checkIfOutside... " << endl;
	
	double m = move.startPos.getX();
	
	// test X
	stepTarget = getCoord(xMax+dist, m, m);
	move.setDirection(stepTarget);
	//note << "My Target: " << stepTarget.info() << endl;
	
	assertTrue("Step: " + borderStep.info(), checkIfOutside(stepTarget, borderStep) == X_BIGGER);
	borderStep = origin;
	
	
	stepTarget = getCoord(xMin-dist, m, m);
	move.setDirection(stepTarget);
	//note << "My Target: " << stepTarget.info() << endl;
	
	assertTrue("Step: " + borderStep.info(), checkIfOutside(stepTarget, borderStep) == X_SMALLER);
	borderStep = origin;
	
	// test Y
	stepTarget = getCoord(m, yMax+dist, m);
	move.setDirection(stepTarget);
	//note << "My Target: " << stepTarget.info() << endl;
	
	assertTrue("Step: " + borderStep.info(), checkIfOutside(stepTarget, borderStep) == Y_BIGGER);
	borderStep = origin;
	
	
	stepTarget = getCoord(m, yMin-dist, m);
	move.setDirection(stepTarget);
	//note << "My Target: " << stepTarget.info() << endl;
	
	assertTrue("Step: " + borderStep.info(), checkIfOutside(stepTarget, borderStep) == Y_SMALLER);
	borderStep = origin;
	
	// test Z
	if (!use2D) {
		
		stepTarget = getCoord(m, m, zMax+dist);
		move.setDirection(stepTarget);
		//note << "My Target: " << stepTarget.info() << endl;
		
		assertTrue("Step: " + borderStep.info(), checkIfOutside(stepTarget, borderStep) == Z_BIGGER);
		borderStep = origin;
		
		
		stepTarget = getCoord(m, m, zMin-dist);
		move.setDirection(stepTarget);
		//note << "My Target: " << stepTarget.info() << endl;
		
		assertTrue("Step: " + borderStep.info(), checkIfOutside(stepTarget, borderStep) == Z_SMALLER);
		borderStep = origin;
			
	}
}

void TestBaseMobilityTest::testComplexCIO()
{
	Coord origin = getCoord(0,0,0);
	Coord borderStep = origin;
	Coord stepTarget;
	double dist = 30.0;
	
	ev << "Testing complex checkIfOutside... " << endl;
	
	double m = move.startPos.getX();
	
	// test X
	//note << endl;
	//note << "Should cross X first." << endl; 
	for (int i = -1; i <= (yMax/dist +1); i++){
		
		for (int j = -1; j <= (zMax/dist +1); j++){
	
			stepTarget = getCoord(xMax+2*dist, i*dist, j*dist);
			move.setDirection(stepTarget);
			//note << "My Target: " << stepTarget.info() << endl;
			
			assertTrue("Step to border: " + borderStep.info(), checkIfOutside(stepTarget, borderStep) == X_BIGGER);
			borderStep = origin;
		}
	}	
	
	for (int i = -1; i <= (yMax/dist +1); i++){
		
		for (int j = -1; j <= (zMax/dist +1); j++){
	
			stepTarget = getCoord(xMin-2*dist, i*dist, j*dist);
			move.setDirection(stepTarget);
			//note << "My Target: " << stepTarget.info() << endl;
			
			assertTrue("Step to border: " + borderStep.info(), checkIfOutside(stepTarget, borderStep) == X_SMALLER);
			borderStep = origin;
		}
	}	
	
	// test Y
	//note << endl;
	//note << "Should cross Y first." << endl; 
	for (int i = -1; i <= (xMax/dist +1); i++){
		
		for (int j = -1; j <= (zMax/dist +1); j++){
	
			stepTarget = getCoord(i*dist, yMax+2*dist, j*dist);
			move.setDirection(stepTarget);
			//note << "My Target: " << stepTarget.info() << endl;
			
			assertTrue("Step to border: " + borderStep.info(), checkIfOutside(stepTarget, borderStep) == Y_BIGGER);
			borderStep = origin;
		}
	}	
	
	for (int i = -1; i <= (xMax/dist +1); i++){
		
		for (int j = -1; j <= (zMax/dist +1); j++){
	
			stepTarget = getCoord(i*dist, yMin-2*dist, j*dist);
			move.setDirection(stepTarget);
			//note << "My Target: " << stepTarget.info() << endl;
			
			assertTrue("Step to border: " + borderStep.info(), checkIfOutside(stepTarget, borderStep) == Y_SMALLER);
			borderStep = origin;
		}
	}
	
	// test Z
	//note << endl;
	//note << "Should cross Z first." << endl; 
	if (!use2D){
		
		for (int i = -1; i <= (xMax/dist +1); i++){
		
			for (int j = -1; j <= (yMax/dist +1); j++){
		
				stepTarget = getCoord(i*dist, j*dist, zMax+2*dist);
				move.setDirection(stepTarget);
				//note << "My Target: " << stepTarget.info() << endl;
				
				assertTrue("Step to border: " + borderStep.info(), checkIfOutside(stepTarget, borderStep) == Z_BIGGER);
				borderStep = origin;
			}
		}
		
		for (int i = -1; i <= (xMax/dist +1); i++){
		
			for (int j = -1; j <= (yMax/dist +1); j++){
		
				stepTarget = getCoord(i*dist, j*dist, zMin-2*dist);
				move.setDirection(stepTarget);
				//note << "My Target: " << stepTarget.info() << endl;
				
				assertTrue("Step to border: " + borderStep.info(), checkIfOutside(stepTarget, borderStep) == Z_SMALLER);
				borderStep = origin;
			}
		}		
	}
}

Coord TestBaseMobilityTest::getCoord(double x, double y, double z) {
	if(use2D) {
		return Coord(x, y);
	} else {
		return Coord(x, y, z);
	}
}

void TestBaseMobilityTest::testBorderCIO()
{	
	Coord origin = getCoord(0,0,0);
	Coord borderStep = origin;
	Coord stepTarget;
	double dist = 30.0;
	
	double min = 0.0;
	int bHandVal;
	
	std::string debug = "";
	
	ev << "Testing border and edges checkIfOutside... " << endl;
	
	double m = move.startPos.getX();
	
	// testing origin
	stepTarget = getCoord(min, min, min);
	move.setDirection(stepTarget);	
	//note << "My Target: " << stepTarget.info() << endl;
				
	assertTrue("Step to border: " + borderStep.info(), checkIfOutside(stepTarget, borderStep) == NOWHERE);
	borderStep = origin;
	
	// "one max corners"
	stepTarget = getCoord(xMax, min, min);
	move.setDirection(stepTarget);	
	//note << "My Target: " << stepTarget.info() << endl;
				
	assertTrue("Step to border: " + borderStep.info(), checkIfOutside(stepTarget, borderStep) == X_BIGGER);
	borderStep = origin;
	
	
	stepTarget = getCoord(min, yMax, min);
	move.setDirection(stepTarget);	
	//note << "My Target: " << stepTarget.info() << endl;
				
	assertTrue("Step to border: " + borderStep.info(), checkIfOutside(stepTarget, borderStep) == Y_BIGGER);
	borderStep = origin;
	
	// "two max corners"
	// this one contains output of the result
	stepTarget = getCoord(xMax, yMax, min);
	move.setDirection(stepTarget);	
	//note << "My Target: " << stepTarget.info() << endl;
				
	assertTrue("Step to border: " + borderStep.info(), (bHandVal = checkIfOutside(stepTarget, borderStep)) == X_BIGGER);
	//setBHandStr(debug, bHandVal);
	//note << "Result: " << debug << endl;
	borderStep = origin;
	
	if(!use2D) {
		stepTarget = getCoord(xMax, min, zMax);
		move.setDirection(stepTarget);	
		//note << "My Target: " << stepTarget.info() << endl;
					
		assertTrue("Step to border: " + borderStep.info(), checkIfOutside(stepTarget, borderStep) == X_BIGGER);
		borderStep = origin;
		
		stepTarget = getCoord(min, min, zMax);
		move.setDirection(stepTarget);	
		//note << "My Target: " << stepTarget.info() << endl;
					
		assertTrue("Step to border: " + borderStep.info(), checkIfOutside(stepTarget, borderStep) == Z_BIGGER);
		borderStep = origin;
		
		
		stepTarget = getCoord(min, yMax, zMax);
		move.setDirection(stepTarget);	
		//note << "My Target: " << stepTarget.info() << endl;
					
		assertTrue("Step to border: " + borderStep.info(), checkIfOutside(stepTarget, borderStep) == Y_BIGGER);
		borderStep = origin;
		
		// "all max corner"
		stepTarget = getCoord(xMax, yMax, zMax);
		move.setDirection(stepTarget);	
		//note << "My Target: " << stepTarget.info() << endl;
					
		assertTrue("Step to border: " + borderStep.info(), checkIfOutside(stepTarget, borderStep) == X_BIGGER);
		borderStep = origin;
	}
}
