#ifndef TESTPHYLAYER_H_
#define TESTPHYLAYER_H_

#include <BasePhyLayer.h>
#include <TestModule.h>
#include "TestDecider.h"

#include <list>

class TestPhyLayer:public BasePhyLayer, public TestModule {
private:

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

	// prepared RSSI mapping for testing purposes
	Mapping* testRSSIMap;

	virtual AnalogueModel* getAnalogueModelFromName(std::string name, ParameterMap& params);

	virtual Decider* getDeciderFromName(std::string name, ParameterMap& params);



public:
	virtual void initialize(int stage);

	virtual void handleMessage(cMessage* msg);

	virtual ~TestPhyLayer();

	void testInitialisation();

};

#endif /*TESTPHYLAYER_H_*/
