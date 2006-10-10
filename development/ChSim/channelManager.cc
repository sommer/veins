/* 
 * Author: Thorsten Pawlak 
 *		Universität Paderborn
 * 
 * Original by: Randy Vu
 *		Technische Universitaet Berlin
 *
 */

#include <omnetpp.h>
#include "channelStateMsg_m.h"

#define EV   ev.disabled() ? (std::ostream&)ev : ev

// module class declaration:
class channelManager : public cSimpleModule
{
	public:
	    Module_Class_Members(channelManager,cSimpleModule,0)
    	virtual void initialize();
    	virtual void handleMessage(cMessage *msg);
    	virtual void finish();
    	
    private:
		int J; // # of ms in cell
		int C; // # of Channels per ms
		int S; // # of Subcarriers in cell	
};

// module type registration:
Define_Module(channelManager);

void channelManager::initialize()
{
	ev << "channelManager->initialize()" << endl;

	J = par("qty_ms");
	C = par("qty_channels");
	S = par("subbands");
}

// implementation of the module class:
void channelManager::handleMessage(cMessage * msg)
{
	if(msg->isName("trigger")) {											// msg is timer event
		delete msg;

		//sent a messages for reach channel of each ms
		for (int j = 0; j < J; j++) {										// to every ms
			for (int c = 0; c < C; ++c) {									// one for each channel of each ms
				channelStateMsg *msgout = new channelStateMsg("channelState");
				msgout->setChannelStateArraySize(S);						// space for SUBBANDS many channelState values in each message
				msgout->setMsId(j);											// number of the ms 0..J
				msgout->setKind(j);											// just for nice colours in the grafical simulation
				msgout->setMsChannel(c);									// number of channel at the ms 0..C*J
				send(msgout, "out", j * C + c);
			}
		}
	}
	else																	// collect incoming messages from ms's
	{
		send(msg, "data_out");
	}
}

void channelManager::finish()
{
	ev << "channelManager->finish()" << endl;
}

