#ifndef TESTMODULE_H_
#define TESTMODULE_H_

#include "TestManager.h"
#include "asserts.h"
#include "OmnetTestBase.h"
#include <FindModule.h>
#include <string>

class TestModule;

/**
 * Class representation of the "assertMessage()" command.
 * It describes the asserted message together with a description
 * text.
 * Normally you shouldn't have to use this class by yourself.
 */
class AssertMessage {
protected:
	std::string msg;
	TestModule* continueModule;
	int continueState;
	/** @brief Stores if this assert executes a test case planned by a
	 * "planTest" call.*/
	bool isPlannedFlag;

public:
	AssertMessage(std::string msg,
				  bool isPlanned = false,
				  TestModule* cModule = 0,
				  int cState = 0):
		msg(msg),
		continueModule(cModule),
		continueState(cState),
		isPlannedFlag(isPlanned)
	{}
	
	virtual ~AssertMessage() {}

	/**
	 * @brief Returns true if this assert executes a previously planned test
	 * case.
	 * @return true if this executes a planned test case
	 */
	virtual bool isPlanned() const { return isPlannedFlag; }
		
	/**
	 * Returns true if the passed message is the message
	 * expected by this AssertMessage.
	 * Has to be implemented by every subclass.
	 */
	virtual bool isMessage(cMessage* msg) = 0;
	
	/**
	 * @brief Returns the message or in case of a planned test the name for this
	 * test case.
	 * @return message or name of test case
	 */
	virtual std::string getMessage() const { return msg; }

	/**
	 * Appends the description text of this AssertMessage
	 * to an outstream.
	 * Should be overriden/extended by subclasses.
	 */
	virtual std::ostream& concat(std::ostream& o) const {
		return o;
	}
	
	/**
	 * Returns the module which waits for the message to 
	 * continue.
	 */
	TestModule* getContinueModule() const {
		return continueModule;
	}
	
	/**
	 * Returns the state of the module to be continued
	 * when the asserted message arrives.
	 */
	int getContinueState() const {
		return continueState;
	}
	
	/**
	 * Needed by the "toString()" method to convert this
	 * class into a string.
	 */
	friend std::ostream& operator<<(std::ostream& o, const AssertMessage& m) {
		return m.concat(o);
	}
};

/**
 * Asserts a message with a specific kind at a specific
 * time.
 * Normally you shouldn't have to use this class by yourself.
 */
class AssertMsgKind:public AssertMessage {
protected:
	int kind;
	simtime_t arrival;
public:
	AssertMsgKind(std::string msg, int kind, simtime_t arrival,
				  bool isPlanned = false,
				  TestModule* cModule = 0,
				  int cState = 0):
		AssertMessage(msg, isPlanned, cModule, cState),
		kind(kind),
		arrival(arrival)
	{}
		
	virtual ~AssertMsgKind() {}
	
	/**
	 * Returns true if the passed message has the kind and
	 * arrival time this AssertMsgKind expects.
	 */
	virtual bool isMessage(cMessage* msg) {
		return msg->getKind() == kind && msg->getArrivalTime() == arrival;
	}
	
	/**
	 * Concatenates the description text together with expected
	 * kind and arrival time to an out stream.
	 */
	virtual std::ostream& concat(std::ostream& o) const{
		o << ": kind = " << kind << ", arrival = " << arrival << "s";
		return o;
	}
};

/**
 * This class provides some testing utilities for Omnet modules.
 * An Omnet modules which extends this class has acces to the following
 * commands:
 * 
 * - assertMessage(description, expected kind, expected arrival)
 * - assertMessage(description, exp. kind, exp. arrival, destination module)
 * 
 * (see doc of these methods for details.)
 * 
 * Note: To be able to work the extending module has to to the following things:
 * 
 * - call method "init(name)" at first initialisation
 * - call method "announceMessage(msg)" at begin of "handleMessage()"
 * - call method "finalize()" at end of "final()"
 * 
 * - TestManager has to be present as global module in simualtion.
 */
class TestModule {
protected:
	std::string name;
	TestManager* manager;
	
	typedef std::list<AssertMessage*> MessageDescList;
	
	MessageDescList expectedMsgs;
	
private:
	
	/**
	 * Proceeds the message assert to the correct destination.
	 */
	void assertNewMessage(AssertMessage* assert, std::string destination);
	
protected:
	/**
	 * @brief Plans test case with the passed name and description.
	 *
	 * The passed name should then be passed to the "testForX()" method call
	 * which actually executes the test case.
	 *
	 * @param name A short identifier for this test case.
	 * @param description A description what this test case covers.
	 */
	void planTest(std::string name, std::string description) {
		manager->planTest(name, description);
	}

	/**
	 * @brief Executes a previously planned test case with the passed name.
	 *
	 * @param name the name of the test case which has been executed.
	 * @return the test message associated with the test case
	 */
	std::string executePlannedTest(std::string name) {
		return manager->executePlannedTest(name);
	}

	/**
	 * This method has to be called by the subclassing Omnet module
	 * to register the TestModule with the TestManager with the
	 * passed name.
	 * 
	 * Note: The name should be unique in simulation.
	 */ 
	void init(const std::string& name);
	
	/**
	 * This method has to be called at the beginning of "handleMessage()"
	 * or whenever a new message arrives at the module.
	 * The passed message should be the newly arrived message.
	 * 
	 * The method checks if the passed message is expected.
	 */
	void announceMessage(cMessage* msg);
	
	/**
	 * This method has to be called at the end of the "finish()"-method.
	 * It checks if there are still message which has been expected but
	 * hadn't arrived.
	 */
	void finalize();
	
	/**
	 * Return a string with the pattern 
	 * "[module name] - passed text"
	 */
	std::string log(std::string msg);
	
	/**
	 * Asserts the arrival of a message described by the passed AssertMessage object 
	 * at the module with the passed name. If the module name is ommited the message is
	 * expected at this module.
	 * This method should be used if you want to write your own AssertMessage-Descriptor.
	 */
	void assertMessage(AssertMessage* assert, std::string destination = "");
	
	
	/**
	 * Asserts the arrival of a message with the specified kind at the specified
	 * time at module with the passed name. If the module name is ommited the message is
	 * expected at this module.
	 */
	void assertMessage(std::string msg, int kind, simtime_t arrival, std::string destination = "");
	
	/**
	 * @brief Analog for "assertMessage" method but for a previously planned
	 * test case.
	 *
	 * @param testName - name of the planned test case this test executes
	 * @param kind - message kind of the expected message
	 * @param arrival - arrival time of the expected message
	 * @param destination - destination where the message is expected
	 * 						if omitted this module is used as destination
	 */
	void testForMessage(std::string testName,
						int kind, simtime_t arrival,
						std::string destination = "");

	/**
	 * Does the same as "assertMessage" plus it calls the "continueTest()"-method
	 * with the passed state as argument when the message arrives.
	 */
	void waitForMessage(int state, std::string msg, int kind, simtime_t arrival, std::string destination = "");
	
	/**
	 * This method is called when a message arrives which ahs been awaited
	 * with the "waitForMessage()"-method.
	 * This method should be implemented by every subclass which wants
	 * to eb able to handle asserted messages.
	 */
	virtual void onAssertedMessage(int state, const cMessage* msg) {};
	
public:
	virtual ~TestModule() {}
};

#endif /*TESTMODULE_H_*/
