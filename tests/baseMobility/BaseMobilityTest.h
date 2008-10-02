#ifndef TEST_BASEMOBILITY_TEST_H_
#define TEST_BASEMOBILITY_TEST_H_

//#define note ev << "[" << getId() << "]: "

#include "BaseMobility.h"
#include "Coord.h"


class BaseMobilityTest : public BaseMobility
{
	
	
	
protected:
	double xMax, yMax, zMax;
	double xMin, yMin, zMin;
	
	bool use2D;
	
	bool allTestsPassed;
	
public:
	//Module_Class_Members( BaseMobilityTest , BaseMobility , 0 );
	
	virtual void initialize(int);
	virtual void finish();

protected:
	Coord pointOutside;
	BorderHandling bHand;
	BorderPolicy bPol;
	
	std::string bPolStr; // name of border policy
	std::string bHandStr; // name of border handling
	
	/** @brief Called upon arrival of a self messages*/
    virtual void handleSelfMsg( cMessage* );
    
    /** @brief Called upon arrival of a border messages*/
    virtual void handleBorderMsg( cMessage* );
    
    void passed(bool);
    
    void setBHandStr(std::string&, int);
	void setBPolStr(std::string&, int);
	
	void testCheckIfOutside();
	void testSimpleCIO();
	void testComplexCIO();
	void testBorderCIO();
	
	void testInitialisation();
	
	Coord getCoord(double x, double y, double z);
	
	void assertTrue(std::string msg, bool value) {
		if (!value) {
			ev << "FAILED: ";
			allTestsPassed = false;
		} else {
			ev << "Passed: ";
		}
		
		ev << msg << std::endl;
	}

	void assertFalse(std::string msg, bool value) { assertTrue(msg, !value); }
	
};

#endif /*TESTHELPERMETHODS_H_*/
