/* -*- mode:c++ -*- ********************************************************
 * file:        NotConnectedRNodePhyLayer.h
 *
 * author:      Karl Wessel
 ***************************************************************************
 * part of:     mixim framework
 * description: application layer which expects to receive and answers at 
				least one broadcast
 ***************************************************************************/


#ifndef NOT_CONNECTED_RNODE_PHY_LAYER_H
#define NOT_CONNECTED_RNODE_PHY_LAYER_H

#include "TestPhyLayer.h"

class NotConnectedRNodePhyLayer : public TestPhyLayer
{
public:
    //Module_Class_Members(NotConnectedRNodePhyLayer, TestPhyLayer, 0);

	bool broadcastReceived;

	virtual void initialize(int stage) {
		TestPhyLayer::initialize(stage);
		if(stage==0){
			broadcastReceived = false;
		}
	}

	virtual void finish() {
		TestPhyLayer::finish();

		assertFalse("Should have received no broadcast.", broadcastReceived);
	}

protected:
	virtual void handleLowerMsg( int srcAddr) {
		ev << "Not Connected R-Node " << myAddr() << ": got broadcast message from " << srcAddr << endl;
		
		broadcastReceived = true;
	}
};

#endif
 
