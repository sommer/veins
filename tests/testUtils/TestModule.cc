#include "TestModule.h"

/**
 * This method has to be called by the subclassing Omnet module
 * to register the TestModule with the TestManager with the
 * passed name.
 * 
 * Note: The name should be unique in simulation.
 */ 
void TestModule::init(const std::string& name) {
	this->name = name;
	
	manager = FindModule<TestManager*>::findGlobalModule();
	
	if(!manager) {
		fail("Could not find TestManager module.");
		exit(1);
	}
	
	manager->registerModule(name, this);
}

/**
 * This method has to be called at the beginning of "handleMessage()"
 * or whenever a new message arrives at the module.
 * The passed message should be the newly arrived message.
 * 
 * The method checks if the passed message is expected.
 */
void TestModule::announceMessage(cMessage* msg) {
	MessageDescList::iterator it = expectedMsgs.begin();
	bool foundMessage = false;
	while(it != expectedMsgs.end()) {
		
		AssertMessage* exp = *it;
		if(exp->isMessage(msg)) {
			if(foundMessage) {
				fail(log("Received message was expected mor then once!"));
			} else {
				pass(log("Expected " + toString(*exp)));
				foundMessage = true;
			}
			delete exp;
			it = expectedMsgs.erase(it);
			continue;				
		}
		it++;
	}
	if(!foundMessage) {
		fail(log("Received not expected Message at " + toString(msg->arrivalTime()) + "s"));
	}
}

/**
 * This method has to be called at the end of the "finish()"-method.
 * It checks if there are still message which has been expected but
 * hadn't arrived.
 */
void TestModule::finalize() {
	for(MessageDescList::iterator it = expectedMsgs.begin();
		it != expectedMsgs.end(); it++) {
		
		AssertMessage* exp = *it;
		fail(log("Expected " + toString(*exp)));
		delete exp;
	}
	
	expectedMsgs.clear();
}

/**
 * Return a string with the pattern 
 * "[module name] - passed text"
 */
std::string TestModule::log(std::string msg) {
	return "[" + name + "] - " + msg;
}	

/**
 * Asserts the arrival of a message with the specified kind at the specified
 * time at this module.
 */
void TestModule::assertMessage(std::string msg, int kind, simtime_t arrival) {
	expectedMsgs.push_back(new AssertMsgKind(msg, kind, arrival));
}

/**
 * Asserts the arrival of a message with the specified kind at the specified
 * time at module with the passed name.
 */
void TestModule::assertMessage(std::string msg, int kind, simtime_t arrival, std::string destination) {
	TestModule* dest = manager->getModule(destination);
	if(!dest) {
		fail(log("No test module with name \"" + destination + "\" found."));
		return;
	}
	
	dest->assertMessage(msg, kind, arrival);
}
