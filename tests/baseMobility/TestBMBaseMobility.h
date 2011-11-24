#ifndef TEST_BM_BASE_MOBILITY_H_
#define TEST_BM_BASE_MOBILITY_H_

#include "BaseMobility.h"


/**
 * @brief This class is intended to test border-handling functionality
 * in BaseMobility in case of 2D and 3D playground.
 * 
 * Therefore the general behaviour of ConstSpeedMobility is used and
 * modified in order to test all possible cases of border-contact.
 *
 * @author Michael Swigulski
 */

class TestBMBaseMobility : public BaseMobility
{

protected:
	// parameters to handle the movement of the host
    
        
    // switches to false if a test failed
    bool testsPassed;
    
    int testStage;
    
    simtime_t expectedBorderTime;
    Move expectedBorderMove;
    
    BorderPolicy policy;
    Coord stepTarget;
    Coord targetPos;
    Coord step; 
    double angle;
    
    Coord expStepTarget;
    Coord expTargetPos;
    Coord expStep; 
    double expAngle;
    
    int ignoreNext;
    
    bool use2D;
    
    double pgsX, pgsY, pgsZ;


public:	
	
	//Module_Class_Members( TestBMBaseMobility , BaseMobility , 0 );
	
	/** @brief Initializes mobility model parameters.*/
    virtual void initialize(int);

	virtual void finish();
	
	
protected:
	    
    
    /** @brief Move the host*/
    virtual void makeMove();

    virtual void fixIfHostGetsOutside();
    
    void assertTrue(std::string msg, bool value) {
		if (!value) {
			ev << "FAILED: ";
			testsPassed = false;
		} else {
			ev << "Passed: ";
		}
		
		ev << msg << std::endl;
	}

	void assertFalse(std::string msg, bool value) { assertTrue(msg, !value); }
	
	void assertRightTime(simtime_t_cref expected);
	
	void assertRightMove(Move& expected);
	
	void assertRightBorderHandling();
	
	void getNextTestMove();
	Coord getCoord(double x, double y, double z);
	
	void calcTestReflection(Coord from, Coord collission, Coord reflected);

};

#endif /*TEST_BM_BASE_MOBILITY_H_*/
