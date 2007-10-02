#ifndef TEST_HELPER_METHODS_H_
#define TEST_HELPER_METHODS_H_

#define note ev << "[" << id() << "]: "

#include "TestBaseMobility.h"
#include "Coord.h"


class TestHelperMethods : public TestBaseMobility
{
	
protected:
	double xMax, yMax, zMax;
	double xMin, yMin, zMin;
	
	bool use2D;
	
public:
	Module_Class_Members( TestHelperMethods , TestBaseMobility , 0 );
	
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
	
	bool testCheckIfOutside();
	bool testSimpleCIO();
	bool testComplexCIO();
	bool testBorderCIO();
	
};

#endif /*TESTHELPERMETHODS_H_*/
