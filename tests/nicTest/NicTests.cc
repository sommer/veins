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

#include "NicTests.h"
#include "TestApp.h"

Define_Module(NicTests);

void NicTests::planTestRun1()
{
	planTestModule("app0", "Receiving Host of pair A");
	planTestModule("app1", "Sending Host of pair A");
	planTestModule("app2", "Receiving Host of pair B");
	planTestModule("app3", "Sending Host of pair B");

	planTest("1.1", "A same channel but different time as B");
	planTest("1.2", "B same channel but different time as A");
	planTest("2.1", "A same channel same time as B");
	planTest("2.2", "B same channel same time as A");
	planTest("3", "Sender of B switches to different channel");
	planTest("4", "B different channel between sender and receiver");
	planTest("5", "Receiver of B switches to different channel");
	planTest("6", "B sender and receiver back to same channel");
	planTest("7.1", "A different channel same time as B");
	planTest("7.2", "B different channel same time as A");
	planTest("8", "B receiver changes channel during reception.");
	planTest("9", "B sender changes channel during transmission.");
}

void NicTests::planTestRun2()
{
	planTestModule("app0", "Receiving Host of pair A");
	planTestModule("app1", "Sending Host of pair A");
	planTestModule("app2", "Receiving Host of pair B");
	planTestModule("app3", "Sending Host of pair B");

	planTest("0.1", "A on Channel 1.");
	planTest("0.2", "B on Channel 1.");
	planTest("1", "Channel usage ratio should be very good (over 1.0).");
}

void NicTests::planTestRun3()
{
	planTestModule("app0", "Receiving Host of pair A");
	planTestModule("app1", "Sending Host of pair A");
	planTestModule("app2", "Receiving Host of pair B");
	planTestModule("app3", "Sending Host of pair B");

	planTest("0.1", "A on Channel 1.");
	planTest("0.2", "B on Channel 2.");
	planTest("1", "Channel usage should be about the same as run 2.");
}

void NicTests::planTestRun4()
{
	planTestModule("app0", "Receiving Host of pair A");
	planTestModule("app1", "Sending Host of pair A");
	planTestModule("app2", "Receiving Host of pair B");
	planTestModule("app3", "Sending Host of pair B");

	planTest("0.1", "A on Channel 1.");
	planTest("0.2", "B on Channel 1.");
	planTest("1", "Channel usage ratio should be very bad (half of run 2).");
}

void NicTests::planTestRun5()
{
	planTestModule("app0", "Receiving Host of pair A");
	planTestModule("app1", "Sending Host of pair A");
	planTestModule("app2", "Receiving Host of pair B");
	planTestModule("app3", "Sending Host of pair B");

	planTest("0.1", "A on Channel 1.");
	planTest("0.2", "B on Channel 2.");
	planTest("1", "Channel usage should be about same as run 2.");
}

void NicTests::testRun1(int stage, cMessage* msg)
{
	if(stage == 0) {
		getModule<TestApp>("app1")->testRun1(stage);
	} else if(stage == 1) {
		getModule<TestApp>("app3")->testRun1(stage);
	} else if(stage == 2) {
		getModule<TestApp>("app1")->testRun1(stage);
	} else if(stage == 3) {
		getModule<TestApp>("app3")->testRun1(stage);
	} else if(stage == 4) {
		getModule<TestApp>("app3")->testRun1(stage);
	} else if(stage == 5) {
		getModule<TestApp>("app2")->testRun1(stage);
		getModule<TestApp>("app3")->testRun1(stage);
	} else if(stage == 6) {
		getModule<TestApp>("app1")->testRun1(stage);
	} else if(stage == 7) {
		getModule<TestApp>("app3")->testRun1(stage);
	} else if(stage == 8) {
		getModule<TestApp>("app3")->testRun1(stage);
	} else if(stage == 9) {
		getModule<TestApp>("app2")->testRun1(stage);
	} else if(stage == 10) {
		getModule<TestApp>("app3")->testRun1(stage);
	} else if(stage == 11) {
		getModule<TestApp>("app3")->testRun1(stage);
	}
	//getModule<TestMacLayer>("mac0")->testRun1(stage, msg);
}

void NicTests::testRun2(int stage, cMessage* msg)
{
	if(stage == 0) {
		TestApp* app1 = getModule<TestApp>("app1");
		TestApp* app2 = getModule<TestApp>("app2");
		TestApp* app3 = getModule<TestApp>("app3");
		TestApp* app0 = getModule<TestApp>("app0");
		assertEqual("Host 0 on channel 1", 1, app0->getCurrentChannel());
		testForEqual("0.1", app0->getCurrentChannel(),
					 	 	app1->getCurrentChannel());
		assertEqual("Host 2 on channel 1", 1, app2->getCurrentChannel());
		testForEqual("0.2", app2->getCurrentChannel(),
					 	 	app3->getCurrentChannel());
		displayPassed = false;
		getModule<TestApp>("app1")->ping(1);
		getModule<TestApp>("app3")->ping(2);
		pingsSent = 2;
	}
}

void NicTests::testRun3(int stage, cMessage* msg)
{
	if(stage == 0) {
		TestApp* app1 = getModule<TestApp>("app1");
		TestApp* app2 = getModule<TestApp>("app2");
		TestApp* app3 = getModule<TestApp>("app3");
		TestApp* app0 = getModule<TestApp>("app0");
		assertEqual("Host 0 on channel 1", 1, app0->getCurrentChannel());
		testForEqual("0.1", app0->getCurrentChannel(),
					 	 	app1->getCurrentChannel());
		assertEqual("Host 2 on channel 2", 2, app2->getCurrentChannel());
		testForEqual("0.2", app2->getCurrentChannel(),
					 	 	app3->getCurrentChannel());
		displayPassed = false;
		getModule<TestApp>("app1")->ping(1);
		getModule<TestApp>("app3")->ping(2);
		pingsSent = 2;
	}
}

void NicTests::testRun4(int stage, cMessage* msg)
{
	if(stage == 0) {
		TestApp* app1 = getModule<TestApp>("app1");
		TestApp* app2 = getModule<TestApp>("app2");
		TestApp* app3 = getModule<TestApp>("app3");
		TestApp* app0 = getModule<TestApp>("app0");
		assertEqual("Host 0 on channel 1", 1, app0->getCurrentChannel());
		testForEqual("0.1", app0->getCurrentChannel(),
					 	 	app1->getCurrentChannel());
		assertEqual("Host 2 on channel 1", 1, app2->getCurrentChannel());
		testForEqual("0.2", app2->getCurrentChannel(),
					 	 	app3->getCurrentChannel());
		displayPassed = false;
		getModule<TestApp>("app1")->ping(1);
		getModule<TestApp>("app3")->ping(2);
		pingsSent = 2;
	}
}

void NicTests::testRun5(int stage, cMessage* msg)
{
	if(stage == 0) {
		TestApp* app1 = getModule<TestApp>("app1");
		TestApp* app2 = getModule<TestApp>("app2");
		TestApp* app3 = getModule<TestApp>("app3");
		TestApp* app0 = getModule<TestApp>("app0");
		assertEqual("Host 0 on channel 1", 1, app0->getCurrentChannel());
		testForEqual("0.1", app0->getCurrentChannel(),
					 	 	app1->getCurrentChannel());
		assertEqual("Host 2 on channel 2", 2, app2->getCurrentChannel());
		testForEqual("0.2", app2->getCurrentChannel(),
					 	 	app3->getCurrentChannel());
		displayPassed = false;
		getModule<TestApp>("app1")->ping(1);
		getModule<TestApp>("app3")->ping(2);
		pingsSent = 2;
	}
}

void NicTests::planTests(int run)
{
	//getModule<TestMacLayer>("mac0")->planTests(run);
    if(run == 1)
    {
        planTestRun1();
    } else if(run == 2) {
    	planTestRun2();
    } else if(run == 3) {
    	planTestRun3();
    } else if(run == 4) {
    	planTestRun4();
    } else if(run == 5) {
    	planTestRun5();
    }
    else
    	assertFalse("Unknown test run number: " + run, true);
}

void NicTests::runTests(int run, int stage, cMessage* msg)
{
	if(run == 1)
	{
		testRun1(stage, msg);
	} else if(run == 2)
	{
		testRun2(stage, msg);
	} else if(run == 3)
	{
		testRun3(stage, msg);
	} else if(run == 4)
	{
		testRun4(stage, msg);
	} else if(run == 5)
	{
		testRun5(stage, msg);
	}
	else
    	assertFalse("Unknown test run number: " + run, true);
}

void NicTests::testForRange(	std::string test,
							simtime_t_cref from, simtime_t_cref to,
							simtime_t_cref act)
{
	std::string testMsg = executePlannedTest(test);
	if(from <= act && act <= to) {
		pass(testMsg + " - ("
			 + toString(from) + "<=" + toString(act) + "<=" + toString(to)
			 + ")");
	} else {
		fail(testMsg + " - ("
			 + toString(from) + "<=" + toString(act) + "<=" + toString(to)
			 + ")");
	}
}

void NicTests::onTestModuleMessage(std::string module, cMessage* msg){
	if(run == 1)
		return;

	if(msg->getKind() >= TestApp::PING && msg->getKind() <= TestApp::PING+1000) {
		pingsRcvd++;

    	if( pingsSent < numPings) {
    		pingsSent++;
    		assertTrue("Less frames received than sent", pingsRcvd < pingsSent);
    		int index = getModule<TestApp>(module)->getParentModule()->getIndex();
    		getModule<TestApp>("app" + toString(index + 1))->ping(pingsSent);
    	}
    	if( pingsRcvd == numPings) {
    		assertEqual("As much pings received as sent.",
    					pingsSent, pingsRcvd);
    		displayPassed = true;
    		double usage = (5000+272+192+304)*pingsRcvd/simTime()/2E+6;

    		switch(run){
    		case 2:
    		case 3:
    		case 5:
    			testForRange("1", 1.0, 2.0, usage);
    			break;
    		case 4:
    			testForRange("1", 0.2, 0.8, usage);
    			break;
    		}
    	}
    } else if(msg->getKind() == BaseMacLayer::PACKET_DROPPED) {
    	if( pingsSent < numPings) {
    		pingsSent++;
    		getModule<TestApp>(module)->ping(pingsSent);
    	}
    }
}
