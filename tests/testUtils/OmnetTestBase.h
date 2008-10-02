#ifndef OMNETTEST_H_
#define OMNETTEST_H_

#include "asserts.h"
#include <omnetpp.h>

class OmnetTestBase:public cSimpleModule {
	
protected:
	virtual void runTests() = 0;
	
public:
	virtual ~OmnetTestBase() {}
	
	virtual void initialize(int stage){
		runTests();
	}
};

#endif /*OMNETTEST_H_*/
