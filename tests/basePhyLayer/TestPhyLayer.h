#ifndef TESTPHYLAYER_H_
#define TESTPHYLAYER_H_

#include <BasePhyLayer.h> 
#include <FWMath.h>
#include <string>

class TestPhyLayer:public BasePhyLayer {
private:
	class TestDecider:public Decider {
		
	};
	
	class TestAnalogueModel:public AnalogueModel {
	public:
		double att;
		
		TestAnalogueModel(double attenuation):
			att(attenuation) {}
		
		Signal& filterSignal(Signal& s) {
			return s;
		}
	};
protected:
	
	void assertTrue(std::string msg, bool value) {
		if (!value) {
			fail(msg);
		} else {
			pass(msg);
		}
	}

	void assertFalse(std::string msg, bool value) { assertTrue(msg, !value); }
	
	void assertEqual(std::string msg, double target, double actual) {
	    if (!FWMath::close(target, actual)) {
	    	fail(msg, target, actual);
		} else {
			pass(msg);
		}
	}
	
	template<class T> void assertEqual(std::string msg, T target, T actual) {
	    if (target != actual) {
	    	fail(msg, target, actual);
		} else {
			pass(msg);
		}
	}
	
	template<class T> void fail(std::string msg, T expected, T actual) {
		ev 	<< "FAILED: " << msg << ": value was '" << actual 
    		<< "' instead of '" << expected << "'" << endl;
	}
	
	void fail(std::string msg) {
		ev << "FAILED: " << msg << std::endl;
	}
	
	void pass(std::string msg) {
		ev << "Passed: " << msg << std::endl;
	}
	
	typedef std::pair<int, std::string> MessageAssert;
	typedef std::multimap<simtime_t, MessageAssert> MessageMap;
	typedef std::pair<simtime_t, MessageAssert> TimedMessage;
	
	MessageMap expectedMsgs;
	
	void assertMessage(std::string msg, int kind, simtime_t arrivalTime) {
		expectedMsgs.insert(TimedMessage(arrivalTime, MessageAssert(kind, msg)));
	}
	
	virtual AnalogueModel* getAnalogueModelFromName(std::string name, ParameterMap& params);		
		
	virtual Decider* getDeciderFromName(std::string name, ParameterMap& params);
	
	void testInitialisation();
	
	void testMacToPhyInterface();
	void testDeciderToPhyInterface();
	
	void testHandleMessage();
public:
	virtual void initialize(int stage);
	
	virtual void handleMessage(cMessage* msg);
};

#endif /*TESTPHYLAYER_H_*/
