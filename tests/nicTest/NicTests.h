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

#ifndef __MIXIM_BASEPHYTESTS_H_
#define __MIXIM_BASEPHYTESTS_H_

#include <omnetpp.h>
#include "../testUtils/TestManager.h"

/**
 * @brief Starts and manages the tests for NIC functionality.
 *
 * Test run 1: tests for Nic80211MultiChannel
 */
class NicTests : public TestManager
{
protected:
	const int numPings;
	int pingsSent;
	int pingsRcvd;
protected:
	/**
	 * @brief Just forwards to the corresponding "planTestRunX()" method.
	 * @param run The test run to plan tests for.
	 */
	virtual void planTests(int run);
	/**
	 * @brief Just forwards to the corresponding "testRunX()" method.
	 * @param run The test run to execute.
	 */
	virtual void runTests(int run, int stage, cMessage* msg);

	/**
	 * @brief Plans tests to be executed in test run 1.
	 */
	void planTestRun1();
	/**
	 * @brief Executes test run 1 by forwarding execution to the correct
	 * TestModule.
	 *
	 * Tests for this test run:
	 * - sending between different host on different channels
	 * - interference between channel (non existent)
	 */
    void testRun1(int stage, cMessage* msg);

    /**
	 * @brief Plans tests to be executed in test run 2.
	 */
	void planTestRun2();
	/**
	 * @brief Executes test run 2 by forwarding execution to the correct
	 * TestModule.
	 *
	 * Tests for this test run:
	 * - communication between two non interfering pairs on same channel
	 */
    void testRun2(int stage, cMessage* msg);

    /**
	 * @brief Plans tests to be executed in test run 3.
	 */
	void planTestRun3();
	/**
	 * @brief Executes test run 3 by forwarding execution to the correct
	 * TestModule.
	 *
	 * Tests for this test run:
	 * - communication between two non interfering pairs on different channel
	 */
    void testRun3(int stage, cMessage* msg);

    /**
	 * @brief Plans tests to be executed in test run 4.
	 */
	void planTestRun4();
	/**
	 * @brief Executes test run 4 by forwarding execution to the correct
	 * TestModule.
	 *
	 * Tests for this test run:
	 * - communication between two interfering pairs on same channel
	 */
    void testRun4(int stage, cMessage* msg);

    /**
	 * @brief Plans tests to be executed in test run 5.
	 */
	void planTestRun5();
	/**
	 * @brief Executes test run 5 by forwarding execution to the correct
	 * TestModule.
	 *
	 * Tests for this test run:
	 * - communication between two interfering pairs on different channel
	 */
    void testRun5(int stage, cMessage* msg);

    void onTestModuleMessage(std::string module, cMessage* msg);

    void testForRange(std::string test,
    				  simtime_t_cref from, simtime_t_cref to, simtime_t_cref act);
public:
    NicTests():
    	numPings(100),
    	pingsSent(0),
    	pingsRcvd(0)
    {}
};

#endif
