#ifndef OMNETTEST_H_
#define OMNETTEST_H_

#include "asserts.h"
#include <omnetpp.h>

class OmnetTestBase:public cSimpleModule {
protected:
	bool testsExecuted;
	
protected:
	virtual void runTests() = 0;
	
public:
	OmnetTestBase()
		:testsExecuted(false)
	{}

	virtual ~OmnetTestBase() {
		displayPassed = false;
		assertTrue("Tests should have been executed!", testsExecuted);
	}
	
	virtual void initialize(int stage){
		runTests();
	}
};

#endif /*OMNETTEST_H_*/
