//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __MIXIM_TESTAPP_H_
#define __MIXIM_TESTAPP_H_

#include <omnetpp.h>
#include "../testUtils/TestModule.h"
#include "BaseModule.h"
#include "Mac80211MultiChannel.h"
#include "SimpleAddress.h"

/**
 * @brief Executes most of the tests by sending packets to other instances of
 * this module.
 */
class TestApp : public BaseModule,
				public TestModule
{
protected:

	cGate*                out;
	int                   myIndex;
	Mac80211MultiChannel* mac;
	int                   pingsSent;
	simtime_t             lastPong;
  protected:
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    void sendPacket(const LAddress::L2Type& dest = LAddress::L2BROADCAST);
    void continueIn(simtime_t time);
    void switchChannel(int channel);

    simtime_t in(simtime_t delta);



  public:
    enum {
    	WAITING = 2224,
		TESTPACKET = 22331,
		PONG = TESTPACKET + 10,
		PING = PONG + 1,

	};
     int getCurrentChannel();
    void testRun1(int stage);
    void startTraffic();
    void ping(int nr);
    void pong();
};

#endif
