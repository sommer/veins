#ifndef TESTMACLAYER_H_
#define TESTMACLAYER_H_

#include <omnetpp.h>
#include "MacToPhyInterface.h"
#include "TestPhyLayer.h"

class TestMacLayer:public cSimpleModule, public TestModule {
protected:
	MacToPhyInterface* phy;
	TestPhyLayer* testPhy;
public:
	virtual void initialize(int stage) {
		if(stage == 0) {
			init("mac" + toString(index()));
			testPhy = FindModule<TestPhyLayer*>::findSubModule(this->parentModule());
		}
	}
		
	virtual void handleMessage(cMessage* msg) {
		announceMessage(msg);
		delete msg;
	}
	
	virtual void finish() {
		finalize();
	}
};

#endif /*TESTMACLAYER_H_*/
