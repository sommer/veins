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
				fail(log("Received message was expected more then once!"));
			} else {
				pass(log("Expected " + toString(*exp)));
				
				TestModule* cont = exp->getContinueModule();
				if(cont) {
					cont->onAssertedMessage(exp->getContinueState(), msg);
				}
				
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
 * Proceeds the message assert to the correct destination.
 */
void TestModule::assertNewMessage(AssertMessage* assert, std::string destination) {
	
	if(destination == "") {
		expectedMsgs.push_back(assert);
	} else {
		TestModule* dest = manager->getModule(destination);
		if(!dest) {
			fail(log("No test module with name \"" + destination + "\" found."));
			return;
		}
		
		dest->expectedMsgs.push_back(assert);
	}
}

/**
 * Asserts the arrival of a message with the specified kind at the specified
 * time at module with the passed name.
 */
void TestModule::assertMessage(	std::string msg, 
								int kind, simtime_t arrival, 
								std::string destination) {
	
	assertNewMessage(new AssertMsgKind(msg, kind, arrival), destination);
}

/**
 * Does the same as "assertMessage" plus it calls the "continueTest()"-method
 * with the passed state as argument when the message arrives.
 */
void TestModule::waitForMessage(int state, std::string msg, 
								int kind, simtime_t arrival, 
								std::string destination) {
	
	assertNewMessage(new AssertMsgKind(msg, kind, arrival, this, state), destination);
}
