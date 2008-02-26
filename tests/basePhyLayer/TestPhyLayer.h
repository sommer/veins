#ifndef TESTPHYLAYER_H_
#define TESTPHYLAYER_H_

#include <BasePhyLayer.h> 
#include <TestModule.h>


class TestPhyLayer:public BasePhyLayer, public TestModule {
private:
	class TestDecider:public Decider {
	public:
		TestDecider(DeciderToPhyInterface* phy):
			Decider::Decider(phy) {}
	};
	
	class TestAnalogueModel:public AnalogueModel {
	public:
		double att;
		
		TestAnalogueModel(double attenuation):
			att(attenuation) {}
		
		void filterSignal(Signal& s) {
			return;
		}
	};
protected:
	int myIndex;
	
	virtual AnalogueModel* getAnalogueModelFromName(std::string name, ParameterMap& params);		
		
	virtual Decider* getDeciderFromName(std::string name, ParameterMap& params);
	
	void testInitialisation();
public:
	virtual void initialize(int stage);
	
	virtual void handleMessage(cMessage* msg);
	
	virtual ~TestPhyLayer();
};

#endif /*TESTPHYLAYER_H_*/
