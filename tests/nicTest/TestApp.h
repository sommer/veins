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

/**
 * @brief Executes most of the tests by sending packets to other instances of
 * this module.
 */
class TestApp : public BaseModule,
				public TestModule
{
protected:
	enum {
		TESTPACKET = 22331,
	};
	cGate* out;
	int myIndex;
	Mac80211MultiChannel* mac;
  protected:
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    void sendPacket(int dest = -1);
    void continueIn(simtime_t time);
    void switchChannel(int channel);
    int getCurrentChannel();
    simtime_t in(simtime_t delta);

  public:
    void testRun1(int stage);
};

#endif
