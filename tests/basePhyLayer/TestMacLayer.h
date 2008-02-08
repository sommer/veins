#ifndef TESTMACLAYER_H_
#define TESTMACLAYER_H_

#include <omnetpp.h>
#include "MacToPhyInterface.h"
#include "TestPhyLayer.h"

class TestMacLayer:public cSimpleModule {
protected:
	MacToPhyInterface* phy;
	TestPhyLayer* testPhy;
public:
	virtual void initialize(int stage) {
		if(stage == 0) {
			testPhy = FindModule<TestPhyLayer*>::findSubModule(this->parentModule());
		}
	}
		
	virtual void handleMessage(cMessage* msg) {
		//TODO: implement
		delete msg;
	}
};

#endif /*TESTMACLAYER_H_*/
