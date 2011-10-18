/* -*- mode:c++ -*- ********************************************************
 * file:        ConnectedRNodePhyLayer.h
 *
 * author:      Karl Wessel
 ***************************************************************************
 * part of:     mixim framework
 * description: physical layer which expects to receive and answers at
				least one broadcast
 ***************************************************************************/


#ifndef CONNECTED_RNODE_PHY_LAYER_H
#define CONNECTED_RNODE_PHY_LAYER_H

#include "CMPhyLayer.h"

using std::endl;

class ConnectedRNodePhyLayer : public CMPhyLayer
{
public:
    //Module_Class_Members(ConnectedRNodePhyLayer, CMPhyLayer, 0);

	bool broadcastReceived;

	virtual void initialize(int stage) {
		CMPhyLayer::initialize(stage);
		if(stage==0){
			broadcastReceived = false;
		}
	}

	virtual void finish() {
		cComponent::finish();

		assertTrue("Should have received at least one broadcast.", broadcastReceived);
	}
protected:
	virtual void handleLowerMsg( const LAddress::L2Type& srcAddr) {
		broadcastReceived = true;

		ev << "Connected R-Node " << myAddr() << ": got broadcast message from " << srcAddr << endl;

		ev << "Connected R-Node " << myAddr() << ": Sending answer packet!" << endl;
		sendDown(srcAddr);
	}
};

#endif

