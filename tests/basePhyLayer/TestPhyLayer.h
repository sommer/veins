#ifndef TESTPHYLAYER_H_
#define TESTPHYLAYER_H_

#include <BasePhyLayer.h> 
#include <TestModule.h>


class TestPhyLayer:public BasePhyLayer, public TestModule {
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
	
	virtual AnalogueModel* getAnalogueModelFromName(std::string name, ParameterMap& params);		
		
	virtual Decider* getDeciderFromName(std::string name, ParameterMap& params);
	
	void testInitialisation();
	
	void testMacToPhyInterface();
	void testDeciderToPhyInterface();
	
	void testHandleMessage();
public:
	virtual void initialize(int stage);
	
	virtual void handleMessage(cMessage* msg);
	
	virtual void finish();
};

#endif /*TESTPHYLAYER_H_*/
