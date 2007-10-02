#ifndef TEST_BM_BASE_MOBILITY_H_
#define TEST_BM_BASE_MOBILITY_H_

#include "TestBaseMobility.h"


/**
 * @brief This class is intended to test border-handling functionality
 * in TestBaseMobility in case of 2D and 3D playground.
 * 
 * Therefore the general behaviour of ConstSpeedMobility is used and
 * modified in order to test all possible cases of border-contact.
 *
 * @author Michael Swigulski
 */

class TestBMBaseMobility : public TestBaseMobility
{

protected:
	// parameters to handle the movement of the host
    
    // brief Size of a step
    Coord stepSize;
    // brief Total number of steps
    int numSteps;
    // Number of steps already moved
    int step;
    
	// coordinate to hold the current target position of the host
    Coord targetPos;
    // coordinate to hold the destination of the current step
    Coord stepTarget;
    
    // switches to false if a test failed
    bool testsPassed;
    
    


public:	
	
	Module_Class_Members( TestBMBaseMobility , TestBaseMobility , 0 );
	
	/** @brief Initializes mobility model parameters.*/
    virtual void initialize(int);

	virtual void finish();
	
	
protected:
    /** @brief Calculate the target position to move to*/
    virtual void setTargetPosition();

    /** @brief Move the host*/
    virtual void makeMove();

    virtual void fixIfHostGetsOutside();
    
    Coord getOutsidePosition();


};

#endif /*TEST_BM_BASE_MOBILITY_H_*/
