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

		void filterSignal(AirFrame*, const Coord&, const Coord&) {
			return;
		}
	};
protected:

	int myIndex;
	int protocolID;

	// prepared RSSI mapping for testing purposes
	Mapping* testRSSIMap;

	virtual AnalogueModel* getAnalogueModelFromName(std::string name, ParameterMap& params);

	virtual Decider* getDeciderFromName(std::string name, ParameterMap& params);

	virtual bool isKnownProtocolId(int id);
	virtual int myProtocolId();

public:
	virtual void initialize(int stage);

	virtual void handleMessage(cMessage* msg);

	virtual ~TestPhyLayer();

	void testInitialisation();

};

#endif /*TESTPHYLAYER_H_*/
