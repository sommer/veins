/* -*- mode:c++ -*- ********************************************************
 * file:        NotConnectedBCNodePhyLayer.h
 *
 * author:      Karl Wessel
 ***************************************************************************
 * part of:     mixim framework
 * description: physical layer which broadcasts packets and expects no
				answer
 ***************************************************************************/


#ifndef NOT_CONNECTED_BCNODE_PHY_LAYER_H
#define NOT_CONNECTED_BCNODE_PHY_LAYER_H

#include "CMPhyLayer.h"

using std::endl;

class NotConnectedBCNodePhyLayer : public CMPhyLayer
{
public:
    //Module_Class_Members(NotConnectedBCNodePhyLayer, CMPhyLayer, 0);

	bool broadcastAnswered;

	virtual void initialize(int stage) {
		CMPhyLayer::initialize(stage);
		if(stage==0){
			broadcastAnswered = false;
#ifdef MIXIM_INET
			scheduleAt(simTime() + 1.0 + (static_cast<double>(myAddr().getInt())) * 0.1, new cMessage(0,10));
#else
			scheduleAt(simTime() + 1.0 + (static_cast<double>(myAddr())) * 0.1, new cMessage(0,10));
#endif
		}
	}

	virtual void finish() {
		cComponent::finish();

		assertFalse("Broadcast should not be answered by any node.",
					broadcastAnswered);
	}
protected:
	virtual void handleLowerMsg(const LAddress::L2Type& srcAddr) {
		broadcastAnswered = true;
		ev << "Not Connected BC-Node " << myAddr() << ": got answer message from " << srcAddr << endl;
	}

	virtual void handleSelfMsg() {
		// we should send a broadcast packet ...
		ev << "Not Connected BC-Node " << myAddr() << ": Sending broadcast packet!" << endl;
		sendDown(LAddress::L2BROADCAST);
	}
};

#endif

