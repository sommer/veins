/* -*- mode:c++ -*- ********************************************************
 * file:        ConnectedBCNodePhyLayer.h
 *
 * author:      Karl Wessel
 ***************************************************************************
 * part of:     mixim framework
 * description: physical test layer which broadcasts packets and expects at least 
				one answer
 ***************************************************************************/


#ifndef CONNECTED_BCNODE_PHY_LAYER_H
#define CONNECTED_BCNODE_PHY_LAYER_H

#include "TestPhyLayer.h"

class ConnectedBCNodePhyLayer : public TestPhyLayer
{
public:
    //Module_Class_Members(ConnectedBCNodePhyLayer, TestPhyLayer, 0);

	bool broadcastAnswered;

	virtual void initialize(int stage) {
		TestPhyLayer::initialize(stage);
		if(stage==0){
			broadcastAnswered = false;
			scheduleAt(simTime() + 1.0 + myAddr() * 0., new cMessage(0,10));
		}
	}

	virtual void finish() {
		TestPhyLayer::finish();

		assertTrue("Broadcast should be answered by at least one node.", 
					broadcastAnswered);
	}
protected:
	virtual void handleLowerMsg(int srcAddr) {
		broadcastAnswered = true;
		ev << "Connected BC-Node " << myAddr() << ": got answer message from " << srcAddr << endl;
	}

	virtual void handleSelfMsg() {
		// we should send a broadcast packet ... 
		ev << "Connected BC-Node " << myAddr() << ": Sending broadcast packet!" << endl;
		sendDown(-1);
	}
};

#endif
 
