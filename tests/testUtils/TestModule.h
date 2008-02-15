#ifndef TESTMODULE_H_
#define TESTMODULE_H_

#include "TestManager.h"
#include "asserts.h"
#include <FindModule.h>
#include <string>

/**
 * Class representation of the "assertMessage()" command.
 * It describes the asserted message together with a description
 * text.
 * Normally you shouldn't have to use this class by yourself.
 */
class AssertMessage {
protected:
	std::string msg;
public:
	AssertMessage(std::string msg):
		msg(msg) {}
		
	/**
	 * Returns true if the passed message is the message
	 * expected by this AssertMessage.
	 * Has to be implemented by every subclass.
	 */
	virtual bool isMessage(cMessage* msg) = 0;
	
	/**
	 * Appends the description text of this AssertMessage
	 * to an outstream.
	 * Should be overriden/extended by subclasses.
	 */
	virtual std::ostream& concat(std::ostream& o) const {
		o << "\"" << msg << "\"";
		return o;
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
	AssertMsgKind(std::string msg, int kind, simtime_t arrival):
		AssertMessage(msg), kind(kind), arrival(arrival) {}
		
	/**
	 * Returns true if the passed message has the kind and
	 * arrival time this AssertMsgKind expects.
	 */
	virtual bool isMessage(cMessage* msg) {
		return msg->kind() == kind && msg->arrivalTime() == arrival;
	}
	
	/**
	 * Concatenates the description text together with expected
	 * kind and arrival time to an out stream.
	 */
	virtual std::ostream& concat(std::ostream& o) const{
		o << "\"" << msg << "\": kind = " << kind << ", arrival = " << arrival << "s";
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
	
protected:
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
	 * Asserts the arrival of a message with the specified kind at the specified
	 * time at this module.
	 */
	void assertMessage(std::string msg, int kind, simtime_t arrival);
	
	/**
	 * Asserts the arrival of a message with the specified kind at the specified
	 * time at module with the passed name.
	 */
	void assertMessage(std::string msg, int kind, simtime_t arrival, std::string destination);
	
};

#endif /*TESTMODULE_H_*/
