#include "TestPhyLayer.h"

Define_Module(TestPhyLayer);

void TestPhyLayer::initialize(int stage) {
	
	//has to be done before decider and analogue models are initialized
	if(stage == 0) 
		myIndex = parentModule()->par("id");
	
	//call BasePhy's initialize
	BasePhyLayer::initialize(stage);
	
	if(stage == 0) {
		
	} else if(stage == 1) {
		
	}
}

void TestPhyLayer::handleMessage(cMessage* msg) {	
	if(msg->kind() == AIR_FRAME) {
		AirFrame* frame = static_cast<AirFrame*>(msg);
		switch(frame->getState()) {
		case FIRST_RECEIVE:
			log("Received new AirFrame (state=FIRST_RECEIVE)");
			if(!usePropagationDelay) {
				log("Since simualtion of propagation delay is disabled we proceed the AirFrame directly to state START_RECEIVE.");
			}
			break;
		case START_RECEIVE:
			log("Received delayed AirFrame (state=START_RECEIVE). Proceeding it directly to RECEIVING state");
			break;
		case RECEIVING:
			log("Received scheduled AirFrame for further processing through the decider.");
			break;
		case END_RECEIVE:
			log("Last receive of scheduled AirFrame because AirFrame transmission is over. (state=END_RECEIVE");
			break;
		default:
			break;
		}
	}
	BasePhyLayer::handleMessage(msg);	
}

void TestPhyLayer::log(std::string msg) {
	ev << "[Host " << myIndex << "] - PhyLayer: " << msg << endl;
}

AnalogueModel* TestPhyLayer::getAnalogueModelFromName(std::string name, ParameterMap& params) {
	cPar par = params["attenuation"];
	return new TestAnalogueModel(name, par.doubleValue(), myIndex);
}
		
Decider* TestPhyLayer::getDeciderFromName(std::string name, ParameterMap& params) {
		
	return new TestDecider(this, myIndex);
}
